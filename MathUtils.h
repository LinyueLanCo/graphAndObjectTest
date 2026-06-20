#pragma once
#include <cmath>

// MathUtils:
// 统一提供游戏内各种复用的数学插值、缓动及运动学公式，均设计为高效率的 inline 模板函数。
namespace MathUtils
{
    // 功能：标准的线性插值 (Linear Interpolation)
    template <typename T>
    inline T lerp(T start, T end, T t)
    {
        return start + (end - start) * t;
    }

    // 功能：指数衰减平滑插值 (Exponential Decay / Smooth To)
    template <typename T>
    inline T smoothTo(T current, T target, T speed)
    {
        return current + (target - current) * speed;
    }

    // 功能：二维向量归一化 (2D Vector Normalization)
    inline void normalize2D(double& x, double& y)
    {
        double length = std::sqrt(x * x + y * y);
        if (length > 1e-6)
        {
            x /= length;
            y /= length;
        }
    }

    // 功能：弹簧阻尼平滑跟随积分器 (Damped Spring Integrator Step)
    // 采用过阻尼设计，逐步更新物理坐标及其速度分量，防止反复振荡
    template <typename T>
    inline void springMove(T& current, T& currentVel, T target, T springFactor, T friction)
    {
        T targetVel = (target - current) * springFactor;
        currentVel = currentVel * friction + targetVel * (1.0 - friction);
        current += currentVel;
    }
}
