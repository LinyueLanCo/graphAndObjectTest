#pragma once

#include <cmath>
#include "Vector2D.h"

// MathUtils:
// 统一提供游戏内各种复用的数学插值、缓动及运动学公式。
namespace MathUtils
{
    // 功能：标准的线性插值 (Linear Interpolation)
    template <typename T>
    inline T lerp(T start, T end, T t)
    {
        return start + (end - start) * t;
    }

    // Vector2D 版本：标量 t 控制二维向量整体插值。
    inline Vector2D lerp(const Vector2D& start, const Vector2D& end, double t)
    {
        return start + (end - start) * t;
    }

    // 功能：指数衰减平滑插值 (Exponential Decay / Smooth To)
    template <typename T>
    inline T smoothTo(T current, T target, T speed)
    {
        return current + (target - current) * speed;
    }

    inline Vector2D smoothTo(const Vector2D& current, const Vector2D& target, double speed)
    {
        return current + (target - current) * speed;
    }

    // 兼容旧调用：原地归一化两个标量分量。
    inline void normalize2D(double& x, double& y)
    {
        Vector2D normalized = Vector2D(x, y).normalized();
        x = normalized.x;
        y = normalized.y;
    }

    // 功能：弹簧阻尼平滑跟随积分器 (Damped Spring Integrator Step)
    template <typename T>
    inline void springMove(T& current, T& currentVel, T target, T springFactor, T friction)
    {
        T targetVel = (target - current) * springFactor;
        currentVel = currentVel * friction + targetVel * (1.0 - friction);
        current += currentVel;
    }

    // Vector2D 版本：二维位置和速度作为一个整体更新。
    inline void springMove(
        Vector2D& current,
        Vector2D& currentVel,
        const Vector2D& target,
        double springFactor,
        double friction
    )
    {
        Vector2D targetVel = (target - current) * springFactor;
        currentVel = currentVel * friction + targetVel * (1.0 - friction);
        current += currentVel;
    }
}
