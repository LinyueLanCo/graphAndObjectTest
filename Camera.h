#pragma once

#include "Config.h"
#include "MathUtils.h"
#include "Vector2D.h"

struct Camera
{
    // 不受世界边界限制的自由相机中心。
    // 跟随、缓动只更新这一层；它表达“相机本来想去哪里”。
    Vector2D logicalCenter;

    // 最终用于 Renderer / WorldToScreen 的相机中心。
    // 这一层由 logicalCenter + 当前 zoom 的世界边界约束得到。
    Vector2D viewCenter;

    double zoom;

    // 逻辑目标中心点：跟随实体位置 + LookOffset。
    Vector2D targetCenter;

    double targetZoom;

    // 自由相机缓动速度。
    Vector2D velocity;

    // ---- 单帧 Camera Debug / 数据流 ----

    // 本帧开始时，Target 与旧 LogicalCenter 之间还差多少。
    Vector2D desiredMove;

    // 自由相机本帧实际缓动了多少（不考虑世界边界）。
    Vector2D logicalDelta;

    // 在“本帧已经采用新 zoom”的前提下，由跟随运动真正造成的 View Camera 位移。
    // 这个值已经扣除了 zoom 改变约束范围造成的重定位，因此可作为视差系统的真实输入。
    Vector2D actualDelta;

    // 最终 View Camera 相对上一帧总共移动了多少。
    // 它包含普通跟随 + zoom/constraint 重定位，不应直接拿去做视差积分。
    Vector2D viewDelta;

    // 仅由 zoom 改变合法视口范围造成的 View Camera 位移。
    Vector2D zoomDelta;

    // 当前边界约束把 View Camera 从 Logical Camera 推开了多少。
    Vector2D constraintOffset;

    // 本帧约束偏移变化量。
    Vector2D constraintDelta;

    // 明确提供给 Parallax System 的位移。
    Vector2D parallaxDelta;

    Camera()
        : logicalCenter(),
        viewCenter(),
        zoom(1.0),
        targetCenter(),
        targetZoom(1.0),
        velocity(),
        desiredMove(),
        logicalDelta(),
        actualDelta(),
        viewDelta(),
        zoomDelta(),
        constraintOffset(),
        constraintDelta(),
        parallaxDelta()
    {
    }

    void resetFrameMotion()
    {
        desiredMove = Vector2D();
        logicalDelta = Vector2D();
        actualDelta = Vector2D();
        viewDelta = Vector2D();
        zoomDelta = Vector2D();
        constraintDelta = Vector2D();
        parallaxDelta = Vector2D();
    }

    // 功能：计算当前 zoom 下屏幕横向覆盖的世界宽度。
    double getVisibleWorldWidth() const
    {
        return WINDOW_WIDTH / zoom;
    }

    // 功能：计算当前 zoom 下屏幕纵向覆盖的世界高度。
    double getVisibleWorldHeight() const
    {
        return WINDOW_HEIGHT / zoom;
    }

    Vector2D getVisibleWorldSize() const
    {
        return Vector2D(getVisibleWorldWidth(), getVisibleWorldHeight());
    }

    // 功能：根据最终 View Center 推导当前视口边界。
    double getViewLeft() const
    {
        return viewCenter.x - getVisibleWorldWidth() / 2.0;
    }

    double getViewRight() const
    {
        return viewCenter.x + getVisibleWorldWidth() / 2.0;
    }

    double getViewBottom() const
    {
        return viewCenter.y - getVisibleWorldHeight() / 2.0;
    }

    double getViewTop() const
    {
        return viewCenter.y + getVisibleWorldHeight() / 2.0;
    }

    // 功能：把任意自由相机中心限制到当前 zoom 下的合法世界视口位置。
    Vector2D calculateConstrainedCenter(
        const Vector2D& source,
        int worldWidth,
        int worldHeight
    ) const
    {
        Vector2D result = source;

        Vector2D visibleSize = getVisibleWorldSize();
        Vector2D halfVisible = visibleSize * 0.5;

        if (worldWidth <= visibleSize.x)
        {
            result.x = worldWidth / 2.0;
        }
        else
        {
            if (result.x < halfVisible.x)
            {
                result.x = halfVisible.x;
            }

            if (result.x > worldWidth - halfVisible.x)
            {
                result.x = worldWidth - halfVisible.x;
            }
        }

        if (worldHeight <= visibleSize.y)
        {
            result.y = worldHeight / 2.0;
        }
        else
        {
            if (result.y < halfVisible.y)
            {
                result.y = halfVisible.y;
            }

            if (result.y > worldHeight - halfVisible.y)
            {
                result.y = worldHeight - halfVisible.y;
            }
        }

        return result;
    }

    // 功能：让相机立即居中跟随目标点，并限制最终 View 在世界范围内。
    void followInstant(
        const Vector2D& targetWorldPosition,
        int worldWidth,
        int worldHeight
    )
    {
        logicalCenter = targetWorldPosition;
        targetCenter = logicalCenter;
        velocity = Vector2D();

        resetFrameMotion();
        limitInWorld(worldWidth, worldHeight);

        constraintOffset = viewCenter - logicalCenter;
    }

    // 兼容旧的 x/y 调用形式。
    void followInstant(
        double targetWorldX,
        double targetWorldY,
        int worldWidth,
        int worldHeight
    )
    {
        followInstant(
            Vector2D(targetWorldX, targetWorldY),
            worldWidth,
            worldHeight
        );
    }

    // 功能：让自由相机平滑跟随目标点，再单独计算最终受边界限制的 View Camera。
    void followSmooth(
        const Vector2D& targetWorldPosition,
        int worldWidth,
        int worldHeight,
        const Vector2D& offsetWorld
    )
    {
        Vector2D oldLogical = logicalCenter;
        Vector2D oldView = viewCenter;
        Vector2D oldConstraintOffset = oldView - oldLogical;

        targetCenter = targetWorldPosition + offsetWorld;
        desiredMove = targetCenter - oldLogical;

        // 先用“旧 Logical Center + 新 Zoom”计算一个基线 View。
        // 这样可以把 zoom 改变合法范围造成的重定位，从普通跟随位移中单独拆出来。
        Vector2D zoomAdjustedView =
            calculateConstrainedCenter(oldLogical, worldWidth, worldHeight);

        zoomDelta = zoomAdjustedView - oldView;

        double springFactor = 0.12;
        double friction = 0.55;

        // 缓动只作用于 Logical Camera，不再让世界边界反向污染跟随状态。
        MathUtils::springMove(
            logicalCenter,
            velocity,
            targetCenter,
            springFactor,
            friction
        );

        // 微距对齐，结束无限逼近。
        if (
            std::fabs(targetCenter.x - logicalCenter.x) < 0.5 &&
            std::fabs(velocity.x) < 0.2
            )
        {
            logicalCenter.x = targetCenter.x;
            velocity.x = 0.0;
        }

        if (
            std::fabs(targetCenter.y - logicalCenter.y) < 0.5 &&
            std::fabs(velocity.y) < 0.2
            )
        {
            logicalCenter.y = targetCenter.y;
            velocity.y = 0.0;
        }

        logicalDelta = logicalCenter - oldLogical;

        // 最终 View Camera 只在这里做一次边界约束。
        limitInWorld(worldWidth, worldHeight);

        // actualDelta：在相同（当前）zoom 约束条件下，真正由跟随造成的可见相机位移。
        actualDelta = viewCenter - zoomAdjustedView;

        // viewDelta：玩家最终在屏幕上看到的总 Camera 位移，包含 zoom/constraint 重定位。
        viewDelta = viewCenter - oldView;

        constraintOffset = viewCenter - logicalCenter;
        constraintDelta = constraintOffset - oldConstraintOffset;

        // 视差只消费真正由跟随造成、且通过边界约束后的 Camera 位移。
        parallaxDelta = actualDelta;
    }

    // 兼容旧的 x/y + offset x/y 调用形式。
    void followSmooth(
        double targetWorldX,
        double targetWorldY,
        int worldWidth,
        int worldHeight,
        double offsetWorldX,
        double offsetWorldY
    )
    {
        followSmooth(
            Vector2D(targetWorldX, targetWorldY),
            worldWidth,
            worldHeight,
            Vector2D(offsetWorldX, offsetWorldY)
        );
    }

    // 功能：设置相机目标缩放值，并限制缩放范围。
    void zoomTo(double newZoom)
    {
        if (newZoom < 0.2)
        {
            newZoom = 0.2;
        }

        if (newZoom > 5.0)
        {
            newZoom = 5.0;
        }

        targetZoom = newZoom;
    }

    // 功能：平滑更新相机当前缩放，使其靠近目标缩放。
    void updateZoom()
    {
        double zoomSpeed = 0.08;
        zoom = MathUtils::smoothTo(zoom, targetZoom, zoomSpeed);
    }

    // 功能：只根据 Logical Camera + 当前 Zoom 计算最终 View Camera。
    // 不再修改 Logical Camera，也不再把撞边界理解为“跟随停止”。
    void limitInWorld(int worldWidth, int worldHeight)
    {
        viewCenter =
            calculateConstrainedCenter(logicalCenter, worldWidth, worldHeight);
    }

    // 功能：把世界坐标转换为 EasyX 屏幕坐标。
    Vector2D worldToScreen(const Vector2D& worldPosition) const
    {
        return Vector2D(
            WINDOW_WIDTH / 2.0 + (worldPosition.x - viewCenter.x) * zoom,
            WINDOW_HEIGHT / 2.0 - (worldPosition.y - viewCenter.y) * zoom
        );
    }

    int worldToScreenX(double worldX) const
    {
        return (int)(
            WINDOW_WIDTH / 2.0 +
            (worldX - viewCenter.x) * zoom
            );
    }

    int worldToScreenY(double worldY) const
    {
        return (int)(
            WINDOW_HEIGHT / 2.0 -
            (worldY - viewCenter.y) * zoom
            );
    }

    // 功能：把世界空间尺寸转换为当前缩放下的屏幕尺寸。
    int worldSizeToScreen(double worldSize) const
    {
        return (int)(worldSize * zoom);
    }
};


extern Camera gCamera;

int worldToScreenX(double worldX);
int worldToScreenY(double worldY);
int worldSizeToScreen(double worldSize);
