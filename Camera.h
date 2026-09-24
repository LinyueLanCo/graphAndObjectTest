#pragma once

#include "Config.h"
#include "MathUtils.h"

struct Camera
{
    // 不受世界边界限制的自由相机中心。
    // 跟随、缓动只更新这一层；它表达“相机本来想去哪里”。
    double logicalCenterX;
    double logicalCenterY;

    // 最终用于 Renderer / WorldToScreen 的相机中心。
    // 这一层由 logicalCenter + 当前 zoom 的世界边界约束得到。
    double centerX;
    double centerY;

    double zoom;

    // 逻辑目标中心点：跟随实体位置 + LookOffset。
    double targetCenterX;
    double targetCenterY;

    double targetZoom;

    // 自由相机缓动速度。
    double vx;
    double vy;

    // ---- 单帧 Camera Debug / 数据流 ----

    // 本帧开始时，Target 与旧 LogicalCenter 之间还差多少。
    double desiredMoveX;
    double desiredMoveY;

    // 自由相机本帧实际缓动了多少（不考虑世界边界）。
    double logicalDx;
    double logicalDy;

    // 在“本帧已经采用新 zoom”的前提下，由跟随运动真正造成的 View Camera 位移。
    // 这个值已经扣除了 zoom 改变约束范围造成的重定位，因此可作为视差系统的真实输入。
    double actualDx;
    double actualDy;

    // 最终 View Camera 相对上一帧总共移动了多少。
    // 它包含普通跟随 + zoom/constraint 重定位，不应直接拿去做视差积分。
    double viewDx;
    double viewDy;

    // 仅由 zoom 改变合法视口范围造成的 View Camera 位移。
    double zoomDx;
    double zoomDy;

    // 当前边界约束把 View Camera 从 Logical Camera 推开了多少。
    double constraintOffsetX;
    double constraintOffsetY;

    // 本帧约束偏移变化量。
    double constraintDx;
    double constraintDy;

    // 明确提供给 Parallax System 的位移。
    double parallaxDx;
    double parallaxDy;

    Camera()
    {
        logicalCenterX = 0.0;
        logicalCenterY = 0.0;

        centerX = 0.0;
        centerY = 0.0;

        zoom = 1.0;

        targetCenterX = 0.0;
        targetCenterY = 0.0;

        targetZoom = 1.0;

        vx = 0.0;
        vy = 0.0;

        resetFrameMotion();
    }

    void resetFrameMotion()
    {
        desiredMoveX = 0.0;
        desiredMoveY = 0.0;

        logicalDx = 0.0;
        logicalDy = 0.0;

        actualDx = 0.0;
        actualDy = 0.0;

        viewDx = 0.0;
        viewDy = 0.0;

        zoomDx = 0.0;
        zoomDy = 0.0;

        constraintOffsetX = 0.0;
        constraintOffsetY = 0.0;

        constraintDx = 0.0;
        constraintDy = 0.0;

        parallaxDx = 0.0;
        parallaxDy = 0.0;
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

    // 功能：根据最终 View Center 推导当前视口边界。
    double getViewLeft() const
    {
        return centerX - getVisibleWorldWidth() / 2.0;
    }

    double getViewRight() const
    {
        return centerX + getVisibleWorldWidth() / 2.0;
    }

    double getViewBottom() const
    {
        return centerY - getVisibleWorldHeight() / 2.0;
    }

    double getViewTop() const
    {
        return centerY + getVisibleWorldHeight() / 2.0;
    }

    // 功能：把任意自由相机中心限制到当前 zoom 下的合法世界视口位置。
    void calculateConstrainedCenter(
        double sourceX,
        double sourceY,
        int worldWidth,
        int worldHeight,
        double& outX,
        double& outY
    ) const
    {
        double visibleW = getVisibleWorldWidth();
        double visibleH = getVisibleWorldHeight();

        double halfW = visibleW / 2.0;
        double halfH = visibleH / 2.0;

        outX = sourceX;
        outY = sourceY;

        if (worldWidth <= visibleW)
        {
            outX = worldWidth / 2.0;
        }
        else
        {
            if (outX < halfW)
            {
                outX = halfW;
            }

            if (outX > worldWidth - halfW)
            {
                outX = worldWidth - halfW;
            }
        }

        if (worldHeight <= visibleH)
        {
            outY = worldHeight / 2.0;
        }
        else
        {
            if (outY < halfH)
            {
                outY = halfH;
            }

            if (outY > worldHeight - halfH)
            {
                outY = worldHeight - halfH;
            }
        }
    }

    // 功能：让相机立即居中跟随目标点，并限制最终 View 在世界范围内。
    void followInstant(double targetWorldX, double targetWorldY, int worldWidth, int worldHeight)
    {
        logicalCenterX = targetWorldX;
        logicalCenterY = targetWorldY;

        targetCenterX = logicalCenterX;
        targetCenterY = logicalCenterY;

        vx = 0.0;
        vy = 0.0;

        resetFrameMotion();

        limitInWorld(worldWidth, worldHeight);

        constraintOffsetX = centerX - logicalCenterX;
        constraintOffsetY = centerY - logicalCenterY;
    }

    // 功能：让自由相机平滑跟随目标点，再单独计算最终受边界限制的 View Camera。
    void followSmooth(
        double targetWorldX,
        double targetWorldY,
        int worldWidth,
        int worldHeight,
        double offsetWorldX,
        double offsetWorldY
    )
    {
        double oldLogicalX = logicalCenterX;
        double oldLogicalY = logicalCenterY;

        double oldViewX = centerX;
        double oldViewY = centerY;

        double oldConstraintOffsetX = oldViewX - oldLogicalX;
        double oldConstraintOffsetY = oldViewY - oldLogicalY;

        // 目标中心点 = 跟随实体位置 + 鼠标观察偏移。
        targetCenterX = targetWorldX + offsetWorldX;
        targetCenterY = targetWorldY + offsetWorldY;

        desiredMoveX = targetCenterX - oldLogicalX;
        desiredMoveY = targetCenterY - oldLogicalY;

        // 先用“旧 Logical Center + 新 Zoom”计算一个基线 View。
        // 这样可以把 zoom 改变合法范围造成的重定位，从普通跟随位移中单独拆出来。
        double zoomAdjustedViewX = oldViewX;
        double zoomAdjustedViewY = oldViewY;

        calculateConstrainedCenter(
            oldLogicalX,
            oldLogicalY,
            worldWidth,
            worldHeight,
            zoomAdjustedViewX,
            zoomAdjustedViewY
        );

        zoomDx = zoomAdjustedViewX - oldViewX;
        zoomDy = zoomAdjustedViewY - oldViewY;

        double springFactor = 0.12;
        double friction = 0.55;

        // 缓动只作用于 Logical Camera，不再让世界边界反向污染跟随状态。
        MathUtils::springMove(logicalCenterX, vx, targetCenterX, springFactor, friction);
        MathUtils::springMove(logicalCenterY, vy, targetCenterY, springFactor, friction);

        // 微距对齐，结束无限逼近。
        if (fabs(targetCenterX - logicalCenterX) < 0.5 && fabs(vx) < 0.2)
        {
            logicalCenterX = targetCenterX;
            vx = 0.0;
        }

        if (fabs(targetCenterY - logicalCenterY) < 0.5 && fabs(vy) < 0.2)
        {
            logicalCenterY = targetCenterY;
            vy = 0.0;
        }

        logicalDx = logicalCenterX - oldLogicalX;
        logicalDy = logicalCenterY - oldLogicalY;

        // 最终 View Camera 只在这里做一次边界约束。
        limitInWorld(worldWidth, worldHeight);

        // actualDelta：在相同（当前）zoom 约束条件下，真正由跟随造成的可见相机位移。
        actualDx = centerX - zoomAdjustedViewX;
        actualDy = centerY - zoomAdjustedViewY;

        // viewDelta：玩家最终在屏幕上看到的总 Camera 位移，包含 zoom/constraint 重定位。
        viewDx = centerX - oldViewX;
        viewDy = centerY - oldViewY;

        constraintOffsetX = centerX - logicalCenterX;
        constraintOffsetY = centerY - logicalCenterY;

        constraintDx = constraintOffsetX - oldConstraintOffsetX;
        constraintDy = constraintOffsetY - oldConstraintOffsetY;

        // 视差只消费真正由跟随造成、且通过边界约束后的 Camera 位移。
        parallaxDx = actualDx;
        parallaxDy = actualDy;
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
        calculateConstrainedCenter(
            logicalCenterX,
            logicalCenterY,
            worldWidth,
            worldHeight,
            centerX,
            centerY
        );
    }

    // 功能：把世界坐标 X 转换为 EasyX 屏幕坐标 X。
    int worldToScreenX(double worldX) const
    {
        return (int)(WINDOW_WIDTH / 2.0 + (worldX - centerX) * zoom);
    }

    // 功能：把世界坐标 Y 转换为 EasyX 屏幕坐标 Y。
    int worldToScreenY(double worldY) const
    {
        return (int)(WINDOW_HEIGHT / 2.0 - (worldY - centerY) * zoom);
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
