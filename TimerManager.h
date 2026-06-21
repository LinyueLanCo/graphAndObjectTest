#pragma once

#include <string>
#include <unordered_map>

// TimerManager：
// 通用计时器管理器，采用状态轮询（Polling）设计。
// 计时器仅保存 { 计时器名字 : 剩余帧数 }。时间走完且被查询（isFinished）后会自动从容器中清除注销。
class TimerManager
{
private:
    std::unordered_map<std::string, double> timers;

public:
    TimerManager();

    // 注册或重置一个计时器
    void setTimer(const std::string& name, double duration);

    // 每一帧推进所有计时器的时间（递减 1.0 帧）
    void update();

    // 查询某个计时器是否已结束。
    // 如果结束了，返回 true，并自动将该计时器从容器中注销，防止重复触发。
    bool isFinished(const std::string& name);

    // 查询某个计时器是否正在运行中（剩余时间 > 0）
    bool isTimerActive(const std::string& name) const;

    // 清除/取消一个计时器
    void clearTimer(const std::string& name);
};
