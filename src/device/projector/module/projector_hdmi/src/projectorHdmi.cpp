#include "projectorHdmi.h"

#include <chrono>

namespace slmaster {
namespace device {

static const char *WINDOW_NAME = "SLMaster_HDMI_Projector";

ProjectorHdmi::ProjectorHdmi()
    : isConnected_(false), state_(ProjectState::Idle), stepRequested_(false),
      currentPatternIndex_(0), screenId_(1), width_(1920), height_(1080),
      exposureTimeUs_(16000), windowName_(WINDOW_NAME), threadRunning_(false),
      isContinue_(true), stepDone_(false) {}

ProjectorHdmi::~ProjectorHdmi() {
    stop();
    disConnect();
}

ProjectorInfo ProjectorHdmi::getInfo() {
    ProjectorInfo info;
    info.dlpEvmType_ = "HDMI";
    info.width_ = width_;
    info.height_ = height_;
    info.isFind_ = isConnected_.load();
    return info;
}

bool ProjectorHdmi::connect() {
    if (isConnected_.load()) {
        return true;
    }

    cv::namedWindow(windowName_, cv::WINDOW_NORMAL);
    cv::setWindowProperty(windowName_, cv::WND_PROP_FULLSCREEN,
                          cv::WINDOW_FULLSCREEN);
    cv::moveWindow(windowName_, screenId_ * width_, 0);

    cv::Mat black = cv::Mat::zeros(height_, width_, CV_8UC1);
    cv::imshow(windowName_, black);
    cv::waitKey(1);

    isConnected_.store(true);
    state_.store(ProjectState::Idle);
    return true;
}

bool ProjectorHdmi::disConnect() {
    if (!isConnected_.load()) {
        return false;
    }

    stop();
    cv::destroyWindow(windowName_);
    cv::waitKey(1);
    isConnected_.store(false);
    return true;
}

bool ProjectorHdmi::isConnect() { return isConnected_.load(); }

bool ProjectorHdmi::populatePatternTableData(
    std::vector<PatternOrderSet> table) {
    std::lock_guard<std::mutex> lock(mutex_);
    patterns_.clear();
    currentPatternIndex_ = 0;

    for (const auto &set : table) {
        for (const auto &img : set.imgs_) {
            cv::Mat resized;
            if (img.cols != width_ || img.rows != height_) {
                cv::resize(img, resized, cv::Size(width_, height_));
            } else {
                resized = img.clone();
            }
            patterns_.push_back(resized);
        }
    }

    return !patterns_.empty();
}

void ProjectorHdmi::projectionLoop() {
    while (threadRunning_.load()) {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock, [this] {
            return !threadRunning_.load() ||
                   state_.load() == ProjectState::Projecting ||
                   stepRequested_.load();
        });

        if (!threadRunning_.load()) {
            break;
        }

        if (patterns_.empty()) {
            state_.store(ProjectState::Idle);
            continue;
        }

        if (stepRequested_.load()) {
            stepRequested_.store(false);
            cv::imshow(windowName_, patterns_[currentPatternIndex_]);
            cv::waitKey(1);
            currentPatternIndex_ =
                (currentPatternIndex_ + 1) % patterns_.size();
            lock.unlock();
            {
                std::lock_guard<std::mutex> stepLock(stepMutex_);
                stepDone_ = true;
            }
            stepDoneCv_.notify_one();
            continue;
        }

        if (state_.load() == ProjectState::Projecting) {
            cv::imshow(windowName_, patterns_[currentPatternIndex_]);
            cv::waitKey(1);

            lock.unlock();
            std::this_thread::sleep_for(
                std::chrono::microseconds(exposureTimeUs_));
            lock.lock();

            currentPatternIndex_++;
            if (currentPatternIndex_ >=
                static_cast<int>(patterns_.size())) {
                if (isContinue_) {
                    currentPatternIndex_ = 0;
                } else {
                    currentPatternIndex_ = 0;
                    state_.store(ProjectState::Idle);
                }
            }
        }
    }
}

bool ProjectorHdmi::project(const bool isContinue) {
    if (!isConnected_.load() || patterns_.empty()) {
        return false;
    }

    stop();

    isContinue_ = isContinue;
    currentPatternIndex_ = 0;
    state_.store(ProjectState::Projecting);
    threadRunning_.store(true);
    projectionThread_ = std::thread(&ProjectorHdmi::projectionLoop, this);

    return true;
}

bool ProjectorHdmi::pause() {
    if (state_.load() != ProjectState::Projecting) {
        return false;
    }
    state_.store(ProjectState::Paused);
    return true;
}

bool ProjectorHdmi::stop() {
    if (!threadRunning_.load()) {
        return true;
    }

    state_.store(ProjectState::Stopped);
    threadRunning_.store(false);
    cv_.notify_all();

    if (projectionThread_.joinable()) {
        projectionThread_.join();
    }

    currentPatternIndex_ = 0;

    if (isConnected_.load()) {
        cv::Mat black = cv::Mat::zeros(height_, width_, CV_8UC1);
        cv::imshow(windowName_, black);
        cv::waitKey(1);
    }

    return true;
}

bool ProjectorHdmi::resume() {
    if (state_.load() != ProjectState::Paused) {
        return false;
    }
    state_.store(ProjectState::Projecting);
    cv_.notify_all();
    return true;
}

bool ProjectorHdmi::step() {
    if (!isConnected_.load() || patterns_.empty()) {
        return false;
    }

    if (!threadRunning_.load()) {
        threadRunning_.store(true);
        state_.store(ProjectState::Idle);
        projectionThread_ = std::thread(&ProjectorHdmi::projectionLoop, this);
    }

    std::unique_lock<std::mutex> stepLock(stepMutex_);
    stepDone_ = false;
    stepRequested_.store(true);
    cv_.notify_all();

    return stepDoneCv_.wait_for(stepLock, std::chrono::milliseconds(1000),
                                [this] { return stepDone_; });
}

bool ProjectorHdmi::isHardwareTriggerSupported() const { return false; }

bool ProjectorHdmi::getLEDCurrent(double &r, double &g, double &b) {
    r = g = b = 0.0;
    return false;
}

bool ProjectorHdmi::setLEDCurrent(const double r, const double g,
                                  const double b) {
    return false;
}

int ProjectorHdmi::getFlashImgsNum() { return 0; }

void ProjectorHdmi::setScreenId(int screenId) { screenId_ = screenId; }

void ProjectorHdmi::setExposureTime(int exposureTimeUs) {
    exposureTimeUs_ = exposureTimeUs;
}

} // namespace device
} // namespace slmaster
