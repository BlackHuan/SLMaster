/**
 * @file dahengCamera.h
 * @author Evans Liu (1369215984@qq.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __DAHENG_CAMERA_H_
#define __DAHENG_CAMERA_H_

#include "GxIAPI.h"
#include "camera.h"
#include "safeQueue.hpp"

#include <atomic>
#include <mutex>
#include <opencv2/opencv.hpp>

/** @brief slmaster */
namespace slmaster {
/** @brief 设备控制库 */
namespace device {
/** @brief 大恒相机控制类 **/
class DEVICE_API DahengCamera : public Camera {
  public:
    explicit DahengCamera(IN const std::string cameraUserId);
    ~DahengCamera();
    CameraInfo getCameraInfo() override;
    bool connect() override;
    bool disConnect() override;
    SafeQueue<cv::Mat> &getImgs() override;
    bool pushImg(IN const cv::Mat &img) override;
    cv::Mat popImg() override;
    bool clearImgs() override;
    bool isConnect() override;
    cv::Mat capture() override;
    bool start() override;
    bool pause() override;
    bool setTrigMode(IN const TrigMode trigMode) override;
    bool setEnumAttribute(IN const std::string attributeName,
                          IN const std::string val) override;
    bool setStringAttribute(IN const std::string attributeName,
                            IN const std::string val) override;
    bool setNumberAttribute(IN const std::string attributeName,
                            IN const double val) override;
    bool setBooleanAttribute(IN const std::string attributeName,
                             IN const bool val) override;
    bool getEnumAttribute(IN const std::string attributeName,
                          OUT std::string &val) override;
    bool getStringAttribute(IN const std::string attributeName,
                            OUT std::string &val) override;
    bool getNumbericalAttribute(IN const std::string attributeName,
                                OUT double &val) override;
    bool getBooleanAttribute(IN const std::string attributeName,
                             OUT bool &val) override;
    int getFps() override;
    GX_DEV_HANDLE getHandle() { return hDevice_; }

  private:
    static std::mutex sLibMutex_;
    static int sLibRefCount_;

    const std::string cameraUserId_;
    GX_DEV_HANDLE hDevice_;
    std::atomic<bool> isOpen_;
    std::atomic<bool> isGrabbing_;
    SafeQueue<cv::Mat> imgs_;
};
} // namespace device
} // namespace slmaster
#endif // __DAHENG_CAMERA_H_
