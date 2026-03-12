/**
 * @file projectorHdmi.h
 * @author Evans Liu (1369215984@qq.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __PROJECTOR_HDMI_H_
#define __PROJECTOR_HDMI_H_

#include "projector.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

/** @brief slmaster **/
namespace slmaster {
/** @brief 设备库 **/
namespace device {
/** @brief HDMI投影仪（通过显示输出投影，无Flash、无LED） */
class DEVICE_API ProjectorHdmi : public Projector {
  public:
    ProjectorHdmi();
    ~ProjectorHdmi();
    /**
     * @brief 获取投影仪信息
     *
     * @return ProjectorInfo 投影仪相关信息
     */
    ProjectorInfo getInfo() override;
    /**
     * @brief 连接（创建全屏投影窗口）
     *
     * @return true 成功
     * @return false 失败
     */
    bool connect() override;
    /**
     * @brief 断开连接（销毁投影窗口）
     *
     * @return true 成功
     * @return false 失败
     */
    bool disConnect() override;
    /**
     * @brief 是否已连接
     *
     * @return true 已连接
     * @return false 未连接
     */
    bool isConnect() override;
    /**
     * @brief 从图案集制作投影序列（缓存到内存）
     *
     * @param table 投影图案集
     */
    bool
    populatePatternTableData(IN std::vector<PatternOrderSet> table) override;
    /**
     * @brief 投影
     *
     * @param isContinue 是否连续投影
     * @return true 成功
     * @return false 失败
     */
    bool project(IN const bool isContinue) override;
    /**
     * @brief 暂停
     *
     * @return true 成功
     * @return false 失败
     */
    bool pause() override;
    /**
     * @brief 停止
     *
     * @return true 成功
     * @return false 失败
     */
    bool stop() override;
    /**
     * @brief 恢复投影
     *
     * @return true 成功
     * @return false 失败
     */
    bool resume() override;
    /**
     * @brief 投影下一帧
     * @warning 仅在步进模式下使用
     *
     * @return true 成功
     * @return false 失败
     */
    bool step() override;
    bool isHardwareTriggerSupported() const override;
    /**
     * @brief 不支持（HDMI投影仪无LED）
     *
     * @return false 始终返回false
     */
    bool getLEDCurrent(OUT double &r, OUT double &g, OUT double &b) override;
    /**
     * @brief 不支持（HDMI投影仪无LED）
     *
     * @return false 始终返回false
     */
    bool setLEDCurrent(IN const double r, IN const double g,
                       IN const double b) override;
    /**
     * @brief 不支持（HDMI投影仪无Flash）
     *
     * @return int 始终返回0
     */
    int getFlashImgsNum() override;

    /**
     * @brief 设置目标屏幕编号
     *
     * @param screenId 屏幕编号（0为主屏幕，1为第二屏幕，以此类推）
     */
    void setScreenId(int screenId);
    /**
     * @brief 设置单帧曝光时间
     *
     * @param exposureTimeUs 曝光时间（微秒）
     */
    void setExposureTime(int exposureTimeUs);

  private:
    enum class ProjectState { Idle, Projecting, Paused, Stopped };

    void projectionLoop();

    std::atomic<bool> isConnected_;
    std::atomic<ProjectState> state_;
    std::atomic<bool> stepRequested_;

    std::vector<cv::Mat> patterns_;
    int currentPatternIndex_;

    int screenId_;
    int width_;
    int height_;
    int exposureTimeUs_;

    std::string windowName_;
    std::thread projectionThread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> threadRunning_;
    bool isContinue_;

    std::mutex stepMutex_;
    std::condition_variable stepDoneCv_;
    bool stepDone_;
};
} // namespace device
} // namespace slmaster

#endif // !__PROJECTOR_HDMI_H_
