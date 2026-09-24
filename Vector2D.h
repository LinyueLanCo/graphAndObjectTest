#pragma once

#include <cmath>

// Vector2D:
// 项目统一使用的基础二维向量类型。
// 只负责二维数学，不包含任何 Gameplay、Camera 或 Collision 语义。
struct Vector2D
{
    double x;
    double y;

    constexpr Vector2D()
        : x(0.0), y(0.0)
    {
    }

    constexpr Vector2D(double newX, double newY)
        : x(newX), y(newY)
    {
    }

    constexpr Vector2D operator+(const Vector2D& rhs) const
    {
        return Vector2D(x + rhs.x, y + rhs.y);
    }

    constexpr Vector2D operator-(const Vector2D& rhs) const
    {
        return Vector2D(x - rhs.x, y - rhs.y);
    }

    constexpr Vector2D operator-() const
    {
        return Vector2D(-x, -y);
    }

    constexpr Vector2D operator*(double scalar) const
    {
        return Vector2D(x * scalar, y * scalar);
    }

    constexpr Vector2D operator/(double scalar) const
    {
        return Vector2D(x / scalar, y / scalar);
    }

    Vector2D& operator+=(const Vector2D& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Vector2D& operator-=(const Vector2D& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    Vector2D& operator*=(double scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2D& operator/=(double scalar)
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    constexpr double lengthSquared() const
    {
        return x * x + y * y;
    }

    double length() const
    {
        return std::sqrt(lengthSquared());
    }

    Vector2D normalized(double tolerance = 1e-6) const
    {
        double len = length();

        if (len <= tolerance)
        {
            return Vector2D();
        }

        return *this / len;
    }

    bool isNearlyZero(double tolerance = 1e-6) const
    {
        return std::fabs(x) <= tolerance && std::fabs(y) <= tolerance;
    }

    static constexpr double dot(const Vector2D& a, const Vector2D& b)
    {
        return a.x * b.x + a.y * b.y;
    }

    static double distance(const Vector2D& a, const Vector2D& b)
    {
        return (b - a).length();
    }
};

inline constexpr Vector2D operator*(double scalar, const Vector2D& vector)
{
    return vector * scalar;
}
