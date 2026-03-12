#include "dahengCamera.h"

#include <chrono>
#include <unordered_map>
#include <vector>

namespace slmaster {
namespace device {

static const std::unordered_map<std::string, GX_FEATURE_ID_CMD> kFloatFeatureMap = {
    {"ExposureTime",                GX_FLOAT_EXPOSURE_TIME},
    {"Gain",                        GX_FLOAT_GAIN},
    {"BlackLevel",                  GX_FLOAT_BLACKLEVEL},
    {"BalanceRatio",                GX_FLOAT_BALANCE_RATIO},
    {"AcquisitionFrameRate",        GX_FLOAT_ACQUISITION_FRAME_RATE},
    {"CurrentAcquisitionFrameRate", GX_FLOAT_CURRENT_ACQUISITION_FRAME_RATE},
    {"TriggerFilterRaising",        GX_FLOAT_TRIGGER_FILTER_RAISING},
    {"TriggerFilterFalling",        GX_FLOAT_TRIGGER_FILTER_FALLING},
    {"TriggerDelay",                GX_FLOAT_TRIGGER_DELAY},
    {"AutoGainMin",                 GX_FLOAT_AUTO_GAIN_MIN},
    {"AutoGainMax",                 GX_FLOAT_AUTO_GAIN_MAX},
    {"AutoExposureTimeMin",         GX_FLOAT_AUTO_EXPOSURE_TIME_MIN},
    {"AutoExposureTimeMax",         GX_FLOAT_AUTO_EXPOSURE_TIME_MAX},
    {"Gamma",                       GX_FLOAT_GAMMA},
    {"Sharpness",                   GX_FLOAT_SHARPNESS},
    {"NoiseReduction",              GX_FLOAT_NOISE_REDUCTION},
    {"PulseWidth",                  GX_FLOAT_PULSE_WIDTH},
    {"DeviceTemperature",           GX_FLOAT_DEVICE_TEMPERATURE},
};

static const std::unordered_map<std::string, GX_FEATURE_ID_CMD> kEnumFeatureMap = {
    {"TriggerMode",         GX_ENUM_TRIGGER_MODE},
    {"TriggerSource",       GX_ENUM_TRIGGER_SOURCE},
    {"TriggerActivation",   GX_ENUM_TRIGGER_ACTIVATION},
    {"TriggerSwitch",       GX_ENUM_TRIGGER_SWITCH},
    {"TriggerSelector",     GX_ENUM_TRIGGER_SELECTOR},
    {"ExposureAuto",        GX_ENUM_EXPOSURE_AUTO},
    {"ExposureMode",        GX_ENUM_EXPOSURE_MODE},
    {"GainAuto",            GX_ENUM_GAIN_AUTO},
    {"GainSelector",        GX_ENUM_GAIN_SELECTOR},
    {"BalanceWhiteAuto",    GX_ENUM_BALANCE_WHITE_AUTO},
    {"BalanceRatioSelector", GX_ENUM_BALANCE_RATIO_SELECTOR},
    {"PixelFormat",         GX_ENUM_PIXEL_FORMAT},
    {"PixelSize",           GX_ENUM_PIXEL_SIZE},
    {"PixelColorFilter",    GX_ENUM_PIXEL_COLOR_FILTER},
    {"TestPattern",         GX_ENUM_TEST_PATTERN},
    {"AcquisitionMode",     GX_ENUM_ACQUISITION_MODE},
    {"UserSetSelector",     GX_ENUM_USER_SET_SELECTOR},
    {"UserSetDefault",      GX_ENUM_USER_SET_DEFAULT},
    {"LineSelector",        GX_ENUM_LINE_SELECTOR},
    {"LineMode",            GX_ENUM_LINE_MODE},
    {"LineSource",          GX_ENUM_LINE_SOURCE},
    {"UserOutputSelector",  GX_ENUM_USER_OUTPUT_SELECTOR},
    {"AcquisitionFrameRateMode", GX_ENUM_ACQUISITION_FRAME_RATE_MODE},
    {"BlackLevelAuto",      GX_ENUM_BLACKLEVEL_AUTO},
    {"BlackLevelSelector",  GX_ENUM_BLACKLEVEL_SELECTOR},
};

static const std::unordered_map<std::string, GX_FEATURE_ID_CMD> kBoolFeatureMap = {
    {"ReverseX",            GX_BOOL_REVERSE_X},
    {"ReverseY",            GX_BOOL_REVERSE_Y},
    {"LineInverter",        GX_BOOL_LINE_INVERTER},
    {"UserOutputValue",     GX_BOOL_USER_OUTPUT_VALUE},
    {"GammaEnable",         GX_BOOL_GAMMA_ENABLE},
    {"LutEnable",           GX_BOOL_LUT_ENABLE},
    {"ChunkModeActive",     GX_BOOL_CHUNKMODE_ACTIVE},
};

static const std::unordered_map<std::string, GX_FEATURE_ID_CMD> kStringFeatureMap = {
    {"DeviceVendorName",    GX_STRING_DEVICE_VENDOR_NAME},
    {"DeviceModelName",     GX_STRING_DEVICE_MODEL_NAME},
    {"DeviceFirmwareVersion", GX_STRING_DEVICE_FIRMWARE_VERSION},
    {"DeviceVersion",       GX_STRING_DEVICE_VERSION},
    {"DeviceSerialNumber",  GX_STRING_DEVICE_SERIAL_NUMBER},
    {"DeviceUserID",        GX_STRING_DEVICE_USERID},
};

static const std::unordered_map<std::string, GX_FEATURE_ID_CMD> kIntFeatureMap = {
    {"Width",               GX_INT_WIDTH},
    {"Height",              GX_INT_HEIGHT},
    {"OffsetX",             GX_INT_OFFSET_X},
    {"OffsetY",             GX_INT_OFFSET_Y},
    {"SensorWidth",         GX_INT_SENSOR_WIDTH},
    {"SensorHeight",        GX_INT_SENSOR_HEIGHT},
    {"BinningHorizontal",   GX_INT_BINNING_HORIZONTAL},
    {"BinningVertical",     GX_INT_BINNING_VERTICAL},
    {"DecimationHorizontal", GX_INT_DECIMATION_HORIZONTAL},
    {"DecimationVertical",  GX_INT_DECIMATION_VERTICAL},
    {"PayloadSize",         GX_INT_PAYLOAD_SIZE},
    {"GevHeartbeatTimeout", GX_INT_GEV_HEARTBEAT_TIMEOUT},
    {"GevPacketSize",       GX_INT_GEV_PACKETSIZE},
    {"GevPacketDelay",      GX_INT_GEV_PACKETDELAY},
    {"AcquisitionSpeedLevel", GX_INT_ACQUISITION_SPEED_LEVEL},
    {"GrayValue",           GX_INT_GRAY_VALUE},
    {"ADCLevel",            GX_INT_ADC_LEVEL},
};

static const std::unordered_map<std::string, GX_FEATURE_ID_CMD> kCommandFeatureMap = {
    {"TriggerSoftware",     GX_COMMAND_TRIGGER_SOFTWARE},
    {"AcquisitionStart",    GX_COMMAND_ACQUISITION_START},
    {"AcquisitionStop",     GX_COMMAND_ACQUISITION_STOP},
    {"UserSetLoad",         GX_COMMAND_USER_SET_LOAD},
    {"UserSetSave",         GX_COMMAND_USER_SET_SAVE},
};

static void GX_STDC onFrameCallback(GX_FRAME_CALLBACK_PARAM *pFrame) {
    if (pFrame == nullptr || pFrame->pUserParam == nullptr)
        return;

    DahengCamera *pCamera = static_cast<DahengCamera *>(pFrame->pUserParam);

    if (pFrame->status != GX_FRAME_STATUS_SUCCESS)
        return;

    cv::Mat img;
    int32_t pixelFormat = pFrame->nPixelFormat;

    if (pixelFormat == GX_PIXEL_FORMAT_MONO8) {
        img = cv::Mat(pFrame->nHeight, pFrame->nWidth, CV_8UC1,
                      const_cast<void *>(pFrame->pImgBuf))
                  .clone();
    } else if (pixelFormat == GX_PIXEL_FORMAT_BAYER_RG8 ||
               pixelFormat == GX_PIXEL_FORMAT_BAYER_GR8 ||
               pixelFormat == GX_PIXEL_FORMAT_BAYER_GB8 ||
               pixelFormat == GX_PIXEL_FORMAT_BAYER_BG8) {
        img = cv::Mat(pFrame->nHeight, pFrame->nWidth, CV_8UC1,
                      const_cast<void *>(pFrame->pImgBuf))
                  .clone();
    } else if (pixelFormat == GX_PIXEL_FORMAT_MONO10 ||
               pixelFormat == GX_PIXEL_FORMAT_MONO12 ||
               pixelFormat == GX_PIXEL_FORMAT_MONO16) {
        img = cv::Mat(pFrame->nHeight, pFrame->nWidth, CV_16UC1,
                      const_cast<void *>(pFrame->pImgBuf))
                  .clone();
    } else if (pixelFormat == GX_PIXEL_FORMAT_RGB8) {
        img = cv::Mat(pFrame->nHeight, pFrame->nWidth, CV_8UC3,
                      const_cast<void *>(pFrame->pImgBuf))
                  .clone();
    } else if (pixelFormat == GX_PIXEL_FORMAT_BGR8) {
        img = cv::Mat(pFrame->nHeight, pFrame->nWidth, CV_8UC3,
                      const_cast<void *>(pFrame->pImgBuf))
                  .clone();
    }

    if (!img.empty()) {
        pCamera->getImgs().push(std::move(img));
    }
}

static bool resolveEnumSymbolic(GX_DEV_HANDLE hDevice,
                                GX_FEATURE_ID_CMD featureID,
                                const std::string &symbolic,
                                int64_t &outValue) {
    uint32_t nEntryNums = 0;
    GX_STATUS status = GXGetEnumEntryNums(hDevice, featureID, &nEntryNums);
    if (status != GX_STATUS_SUCCESS || nEntryNums == 0)
        return false;

    size_t bufSize = nEntryNums * sizeof(GX_ENUM_DESCRIPTION);
    std::vector<GX_ENUM_DESCRIPTION> entries(nEntryNums);
    status = GXGetEnumDescription(hDevice, featureID, entries.data(), &bufSize);
    if (status != GX_STATUS_SUCCESS)
        return false;

    for (uint32_t i = 0; i < nEntryNums; ++i) {
        if (symbolic == entries[i].szSymbolic) {
            outValue = entries[i].nValue;
            return true;
        }
    }
    return false;
}

static bool resolveEnumValue(GX_DEV_HANDLE hDevice,
                             GX_FEATURE_ID_CMD featureID, int64_t value,
                             std::string &outSymbolic) {
    uint32_t nEntryNums = 0;
    GX_STATUS status = GXGetEnumEntryNums(hDevice, featureID, &nEntryNums);
    if (status != GX_STATUS_SUCCESS || nEntryNums == 0)
        return false;

    size_t bufSize = nEntryNums * sizeof(GX_ENUM_DESCRIPTION);
    std::vector<GX_ENUM_DESCRIPTION> entries(nEntryNums);
    status = GXGetEnumDescription(hDevice, featureID, entries.data(), &bufSize);
    if (status != GX_STATUS_SUCCESS)
        return false;

    for (uint32_t i = 0; i < nEntryNums; ++i) {
        if (value == entries[i].nValue) {
            outSymbolic = entries[i].szSymbolic;
            return true;
        }
    }
    return false;
}

DahengCamera::DahengCamera(const std::string cameraUserId)
    : cameraUserId_(cameraUserId), hDevice_(nullptr), isOpen_(false),
      isGrabbing_(false) {
    GXInitLib();
}

DahengCamera::~DahengCamera() {
    if (isGrabbing_) {
        pause();
    }
    if (isOpen_) {
        disConnect();
    }
    GXCloseLib();
}

CameraInfo DahengCamera::getCameraInfo() {
    CameraInfo info;
    info.isFind_ = false;

    uint32_t nDeviceNum = 0;
    GX_STATUS status = GXUpdateDeviceList(&nDeviceNum, 1000);
    if (status != GX_STATUS_SUCCESS || nDeviceNum == 0) {
        return info;
    }

    size_t bufSize = nDeviceNum * sizeof(GX_DEVICE_BASE_INFO);
    std::vector<GX_DEVICE_BASE_INFO> deviceInfos(nDeviceNum);
    status = GXGetAllDeviceBaseInfo(deviceInfos.data(), &bufSize);
    if (status != GX_STATUS_SUCCESS) {
        return info;
    }

    for (uint32_t i = 0; i < nDeviceNum; ++i) {
        if (cameraUserId_ == deviceInfos[i].szUserID) {
            info.isFind_ = true;
            info.cameraKey_ = deviceInfos[i].szSN;
            info.cameraUserId_ = deviceInfos[i].szUserID;
            switch (deviceInfos[i].deviceClass) {
            case GX_DEVICE_CLASS_USB2:
                info.deviceType_ = "USB2";
                break;
            case GX_DEVICE_CLASS_GEV:
                info.deviceType_ = "GigE";
                break;
            case GX_DEVICE_CLASS_U3V:
                info.deviceType_ = "USB3";
                break;
            default:
                info.deviceType_ = "Unknown";
                break;
            }
            break;
        }
    }
    return info;
}

bool DahengCamera::connect() {
    uint32_t nDeviceNum = 0;
    GX_STATUS status = GXUpdateDeviceList(&nDeviceNum, 1000);
    if (status != GX_STATUS_SUCCESS || nDeviceNum == 0) {
        printf("Daheng: no device found, ErrorCode[%d]\n", status);
        return false;
    }

    GX_OPEN_PARAM openParam;
    openParam.pszContent = const_cast<char *>(cameraUserId_.c_str());
    openParam.openMode = GX_OPEN_USERID;
    openParam.accessMode = GX_ACCESS_EXCLUSIVE;

    status = GXOpenDevice(&openParam, &hDevice_);
    if (status != GX_STATUS_SUCCESS) {
        printf("Daheng: open device failed! userId[%s], ErrorCode[%d]\n",
               cameraUserId_.c_str(), status);
        hDevice_ = nullptr;
        return false;
    }

    isOpen_ = true;
    return true;
}

bool DahengCamera::disConnect() {
    if (!hDevice_) {
        printf("Daheng: close camera fail. No camera.\n");
        return false;
    }

    if (!isOpen_) {
        printf("Daheng: camera is already closed.\n");
        return false;
    }

    if (isGrabbing_) {
        pause();
    }

    GX_STATUS status = GXCloseDevice(hDevice_);
    if (status != GX_STATUS_SUCCESS) {
        printf("Daheng: close device failed! ErrorCode[%d]\n", status);
        return false;
    }

    hDevice_ = nullptr;
    isOpen_ = false;
    return true;
}

SafeQueue<cv::Mat> &DahengCamera::getImgs() { return imgs_; }

bool DahengCamera::pushImg(const cv::Mat &img) {
    imgs_.push(std::move(img));
    return true;
}

cv::Mat DahengCamera::popImg() {
    cv::Mat img;
    imgs_.try_move_pop(img);
    return img;
}

bool DahengCamera::clearImgs() {
    SafeQueue<cv::Mat> emptyQueue;
    imgs_.swap(emptyQueue);
    return true;
}

bool DahengCamera::isConnect() { return isOpen_ && hDevice_ != nullptr; }

cv::Mat DahengCamera::capture() {
    if (!isConnect()) {
        printf("Daheng: camera is not open!\n");
        return cv::Mat();
    }

    const int preNums = imgs_.size();

    setTrigMode(TrigMode::trigSoftware);

    GX_STATUS status = GXSendCommand(hDevice_, GX_COMMAND_TRIGGER_SOFTWARE);
    if (status != GX_STATUS_SUCCESS) {
        printf("Daheng: software trigger fail, ErrorCode[%d]\n", status);
        return cv::Mat();
    }

    double exposureTime = 100000.0;
    getNumbericalAttribute("ExposureTime", exposureTime);

    auto timeBegin = std::chrono::system_clock::now();
    while (preNums == (int)imgs_.size()) {
        auto timeEnd = std::chrono::system_clock::now();
        auto timeElapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(timeEnd -
                                                                  timeBegin)
                .count() *
            (double)std::chrono::milliseconds::period::num /
            std::chrono::milliseconds::period::den;
        if (timeElapsed > (exposureTime / 1000000.0 * 2)) {
            break;
        }
    }

    cv::Mat capturedImg = imgs_.back();

    setTrigMode(TrigMode::trigLine);

    return capturedImg;
}

bool DahengCamera::start() {
    if (!isConnect()) {
        printf("Daheng: camera is not open!\n");
        return false;
    }

    if (isGrabbing_) {
        printf("Daheng: camera is already grabbing.\n");
        return false;
    }

    GX_STATUS status =
        GXRegisterCaptureCallback(hDevice_, this, onFrameCallback);
    if (status != GX_STATUS_SUCCESS) {
        printf("Daheng: register capture callback failed! ErrorCode[%d]\n",
               status);
        return false;
    }

    status = GXSendCommand(hDevice_, GX_COMMAND_ACQUISITION_START);
    if (status != GX_STATUS_SUCCESS) {
        printf("Daheng: start grabbing failed! ErrorCode[%d]\n", status);
        GXUnregisterCaptureCallback(hDevice_);
        return false;
    }

    isGrabbing_ = true;
    return true;
}

bool DahengCamera::pause() {
    if (!isConnect()) {
        printf("Daheng: camera is not open!\n");
        return false;
    }

    if (!isGrabbing_) {
        printf("Daheng: camera is not grabbing.\n");
        return false;
    }

    GX_STATUS status = GXSendCommand(hDevice_, GX_COMMAND_ACQUISITION_STOP);
    if (status != GX_STATUS_SUCCESS) {
        printf("Daheng: stop grabbing failed! ErrorCode[%d]\n", status);
        return false;
    }

    status = GXUnregisterCaptureCallback(hDevice_);
    if (status != GX_STATUS_SUCCESS) {
        printf("Daheng: unregister capture callback failed! ErrorCode[%d]\n",
               status);
        return false;
    }

    isGrabbing_ = false;
    return true;
}

bool DahengCamera::setTrigMode(const TrigMode trigMode) {
    if (!isConnect()) {
        printf("Daheng: camera is not open!\n");
        return false;
    }

    GX_STATUS status = GX_STATUS_SUCCESS;

    if (trigContinous == trigMode) {
        status = GXSetEnum(hDevice_, GX_ENUM_TRIGGER_MODE,
                           GX_TRIGGER_MODE_OFF);
        if (status != GX_STATUS_SUCCESS) {
            printf("Daheng: set TriggerMode Off fail, ErrorCode[%d]\n",
                   status);
            return false;
        }
    } else if (trigSoftware == trigMode) {
        status =
            GXSetEnum(hDevice_, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_ON);
        if (status != GX_STATUS_SUCCESS) {
            printf("Daheng: set TriggerMode On fail, ErrorCode[%d]\n", status);
            return false;
        }

        status = GXSetEnum(hDevice_, GX_ENUM_TRIGGER_SOURCE,
                           GX_TRIGGER_SOURCE_SOFTWARE);
        if (status != GX_STATUS_SUCCESS) {
            printf("Daheng: set TriggerSource Software fail, ErrorCode[%d]\n",
                   status);
            return false;
        }
    } else if (trigLine == trigMode) {
        status =
            GXSetEnum(hDevice_, GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_ON);
        if (status != GX_STATUS_SUCCESS) {
            printf("Daheng: set TriggerMode On fail, ErrorCode[%d]\n", status);
            return false;
        }

        status = GXSetEnum(hDevice_, GX_ENUM_TRIGGER_SOURCE,
                           GX_TRIGGER_SOURCE_LINE0);
        if (status != GX_STATUS_SUCCESS) {
            printf("Daheng: set TriggerSource Line0 fail, ErrorCode[%d]\n",
                   status);
            return false;
        }
    }

    return true;
}

bool DahengCamera::setEnumAttribute(const std::string attributeName,
                                    const std::string val) {
    if (!isConnect()) {
        printf("Daheng: camera is not open!\n");
        return false;
    }

    auto it = kEnumFeatureMap.find(attributeName);
    if (it == kEnumFeatureMap.end()) {
        printf("Daheng: unknown enum attribute [%s]\n", attributeName.c_str());
        return false;
    }

    int64_t enumValue = 0;
    if (!resolveEnumSymbolic(hDevice_, it->second, val, enumValue)) {
        printf("Daheng: cannot resolve enum symbolic [%s] for [%s]\n",
               val.c_str(), attributeName.c_str());
        return false;
    }

    return GXSetEnum(hDevice_, it->second, enumValue) == GX_STATUS_SUCCESS;
}

bool DahengCamera::setStringAttribute(const std::string attributeName,
                                      const std::string val) {
    if (!isConnect()) {
        printf("Daheng: camera is not open!\n");
        return false;
    }

    auto it = kStringFeatureMap.find(attributeName);
    if (it == kStringFeatureMap.end()) {
        printf("Daheng: unknown string attribute [%s]\n",
               attributeName.c_str());
        return false;
    }

    return GXSetString(hDevice_, it->second,
                       const_cast<char *>(val.c_str())) == GX_STATUS_SUCCESS;
}

bool DahengCamera::setNumberAttribute(const std::string attributeName,
                                      const double val) {
    if (!isConnect()) {
        printf("Daheng: camera is not open!\n");
        return false;
    }

    auto itFloat = kFloatFeatureMap.find(attributeName);
    if (itFloat != kFloatFeatureMap.end()) {
        return GXSetFloat(hDevice_, itFloat->second, val) ==
               GX_STATUS_SUCCESS;
    }

    auto itInt = kIntFeatureMap.find(attributeName);
    if (itInt != kIntFeatureMap.end()) {
        return GXSetInt(hDevice_, itInt->second, static_cast<int64_t>(val)) ==
               GX_STATUS_SUCCESS;
    }

    printf("Daheng: unknown number attribute [%s]\n", attributeName.c_str());
    return false;
}

bool DahengCamera::setBooleanAttribute(const std::string attributeName,
                                       const bool val) {
    if (!isConnect()) {
        printf("Daheng: camera is not open!\n");
        return false;
    }

    auto it = kBoolFeatureMap.find(attributeName);
    if (it == kBoolFeatureMap.end()) {
        printf("Daheng: unknown bool attribute [%s]\n", attributeName.c_str());
        return false;
    }

    return GXSetBool(hDevice_, it->second, val) == GX_STATUS_SUCCESS;
}

bool DahengCamera::getEnumAttribute(const std::string attributeName,
                                    std::string &val) {
    if (!isConnect()) {
        printf("Daheng: camera is not open!\n");
        return false;
    }

    auto it = kEnumFeatureMap.find(attributeName);
    if (it == kEnumFeatureMap.end()) {
        printf("Daheng: unknown enum attribute [%s]\n", attributeName.c_str());
        return false;
    }

    int64_t enumValue = 0;
    GX_STATUS status = GXGetEnum(hDevice_, it->second, &enumValue);
    if (status != GX_STATUS_SUCCESS) {
        printf("Daheng: get enum [%s] fail, ErrorCode[%d]\n",
               attributeName.c_str(), status);
        return false;
    }

    if (!resolveEnumValue(hDevice_, it->second, enumValue, val)) {
        val = std::to_string(enumValue);
    }

    return true;
}

bool DahengCamera::getStringAttribute(const std::string attributeName,
                                      std::string &val) {
    if (!isConnect()) {
        printf("Daheng: camera is not open!\n");
        return false;
    }

    auto it = kStringFeatureMap.find(attributeName);
    if (it == kStringFeatureMap.end()) {
        printf("Daheng: unknown string attribute [%s]\n",
               attributeName.c_str());
        return false;
    }

    size_t nSize = 0;
    GX_STATUS status = GXGetStringLength(hDevice_, it->second, &nSize);
    if (status != GX_STATUS_SUCCESS || nSize == 0) {
        return false;
    }

    std::vector<char> buf(nSize);
    status = GXGetString(hDevice_, it->second, buf.data(), &nSize);
    if (status != GX_STATUS_SUCCESS) {
        return false;
    }

    val = buf.data();
    return true;
}

bool DahengCamera::getNumbericalAttribute(const std::string attributeName,
                                          double &val) {
    if (!isConnect()) {
        printf("Daheng: camera is not open!\n");
        return false;
    }

    auto itFloat = kFloatFeatureMap.find(attributeName);
    if (itFloat != kFloatFeatureMap.end()) {
        return GXGetFloat(hDevice_, itFloat->second, &val) ==
               GX_STATUS_SUCCESS;
    }

    auto itInt = kIntFeatureMap.find(attributeName);
    if (itInt != kIntFeatureMap.end()) {
        int64_t intVal = 0;
        GX_STATUS status = GXGetInt(hDevice_, itInt->second, &intVal);
        if (status == GX_STATUS_SUCCESS) {
            val = static_cast<double>(intVal);
            return true;
        }
        return false;
    }

    printf("Daheng: unknown number attribute [%s]\n", attributeName.c_str());
    return false;
}

bool DahengCamera::getBooleanAttribute(const std::string attributeName,
                                       bool &val) {
    if (!isConnect()) {
        printf("Daheng: camera is not open!\n");
        return false;
    }

    auto it = kBoolFeatureMap.find(attributeName);
    if (it == kBoolFeatureMap.end()) {
        printf("Daheng: unknown bool attribute [%s]\n", attributeName.c_str());
        return false;
    }

    return GXGetBool(hDevice_, it->second, &val) == GX_STATUS_SUCCESS;
}

int DahengCamera::getFps() {
    double fps = 0.0;
    GXGetFloat(hDevice_, GX_FLOAT_CURRENT_ACQUISITION_FRAME_RATE, &fps);
    return static_cast<int>(fps);
}

} // namespace device
} // namespace slmaster
