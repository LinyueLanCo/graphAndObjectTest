#include "TimerManager.h"

TimerManager::TimerManager()
{
    timers.clear();
}

void TimerManager::setTimer(const std::string& name, double duration)
{
    timers[name] = duration;
}

void TimerManager::update()
{
    for (auto& pair : timers)
    {
        if (pair.second > 0.0)
        {
            pair.second -= 1.0;
        }
    }
}

bool TimerManager::isFinished(const std::string& name)
{
    auto it = timers.find(name);
    if (it != timers.end())
    {
        if (it->second <= 0.0)
        {
            timers.erase(it); // 时间到了，自动擦除回收
            return true;
        }
    }
    return false;
}

bool TimerManager::isTimerActive(const std::string& name) const
{
    auto it = timers.find(name);
    if (it != timers.end())
    {
        return it->second > 0.0;
    }
    return false;
}

void TimerManager::clearTimer(const std::string& name)
{
    timers.erase(name);
}
