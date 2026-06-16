#pragma once

#include "Config.h"

struct Camera
{

    // 逻辑用：摄像机真正跟随和平滑的中心点。
    // 视口边界由这个中心点和 zoom 临时推导，避免平滑缩放时左下角锚点漂移。
    double centerX;
    double centerY;

    double zoom;

    // 逻辑目标中心点。
    double targetCenterX;
    double targetCenterY;

    double targetZoom;

    // 功能：初始化相机位置、目标位置和缩放参数。
    Camera()
    {
        centerX = 0;
        centerY = 0;

        zoom = 1.0;

        targetCenterX = 0;
        targetCenterY = 0;

        targetZoom = 1.0;
    }

    // 功能：计算当前 zoom 下屏幕横向覆盖的世界宽度。
    double getVisibleWorldWidth() const
    {
        // 用固定窗口宽度除以缩放倍率，得到当前逻辑视口宽度。
        return WINDOW_WIDTH / zoom;
    }

    // 功能：计算当前 zoom 下屏幕纵向覆盖的世界高度。
    double getVisibleWorldHeight() const
    {
        // 用固定窗口高度除以缩放倍率，得到当前逻辑视口高度。
        return WINDOW_HEIGHT / zoom;
    }


    // 功能：根据中心点和可见宽度推导当前视口左边界。
    double getViewLeft() const
    {
        return centerX - getVisibleWorldWidth() / 2.0;
    }

    // 功能：根据中心点和可见宽度推导当前视口右边界。
    double getViewRight() const
    {
        return centerX + getVisibleWorldWidth() / 2.0;
    }

    // 功能：根据中心点和可见高度推导当前视口下边界。
    double getViewBottom() const
    {
        return centerY - getVisibleWorldHeight() / 2.0;
    }

    // 功能：根据中心点和可见高度推导当前视口上边界。
    double getViewTop() const
    {
        return centerY + getVisibleWorldHeight() / 2.0;
    }





    // 功能：让相机立即居中跟随目标点，并限制在世界范围内。
    void followInstant(double targetWorldX, double targetWorldY, int worldWidth, int worldHeight)
    {
        centerX = targetWorldX;
        centerY = targetWorldY;

        targetCenterX = centerX;
        targetCenterY = centerY;

        limitInWorld(worldWidth, worldHeight);
    }

    // 功能：让相机平滑跟随目标点，并叠加鼠标观察偏移。
    void followSmooth(
        double targetWorldX,
        double targetWorldY,
        int worldWidth,
        int worldHeight,
        double offsetWorldX,
        double offsetWorldY
    )
    {
        // 目标中心点 = 跟随实体位置 + 鼠标观察偏移。
        // 注意：这里不再减 visibleW / 2，也不再减 visibleH / 2。
        targetCenterX = targetWorldX + offsetWorldX;
        targetCenterY = targetWorldY + offsetWorldY;

        double followSpeed = 0.16;

        // 用中心点与目标中心点的差值乘以跟随系数，得到本帧平滑位移。
        centerX += (targetCenterX - centerX) * followSpeed;
        centerY += (targetCenterY - centerY) * followSpeed;

        limitInWorld(worldWidth, worldHeight);
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
        // 用当前缩放和目标缩放的差值乘以速度，得到本帧缩放变化量。
        zoom += (targetZoom - zoom) * zoomSpeed;
    }

    // 将摄像机视口限制在世界范围内。
    // 这里修正的是摄像机中心点，而不是旧的视口左下角。
    // 如果世界尺寸小于可见视口，就让摄像机中心落在世界中心。
    // 功能：把相机中心限制在关卡世界边界允许的可见范围内。
    void limitInWorld(int worldWidth, int worldHeight)
    {
        double visibleW = getVisibleWorldWidth();
        double visibleH = getVisibleWorldHeight();

        double halfW = visibleW / 2.0;
        double halfH = visibleH / 2.0;
        //如果世界宽度小于等于可见宽度，摄像机中心 X 固定在世界中心；否则限制在半屏范围内。
        if (worldWidth <= visibleW)
        {
            centerX = worldWidth / 2.0;
        }
        else
        {
            if (centerX < halfW)
            {
                centerX = halfW;
            }

            if (centerX > worldWidth - halfW)
            {
                centerX = worldWidth - halfW;
            }
        }

        if (worldHeight <= visibleH)
        {
            centerY = worldHeight / 2.0;
        }
        else
        {
            if (centerY < halfH)
            {
                centerY = halfH;
            }

            if (centerY > worldHeight - halfH)
            {
                centerY = worldHeight - halfH;
            }
        }
    }

    // 功能：把世界坐标 X 转换为 EasyX 屏幕坐标 X。
    int worldToScreenX(double worldX) const
    {
        // 以屏幕中心为锚点，把世界点相对摄像机中心的距离乘以 zoom。
        return (int)(WINDOW_WIDTH / 2.0 + (worldX - centerX) * zoom);
    }

    // 功能：把世界坐标 Y 转换为 EasyX 屏幕坐标 Y。
    int worldToScreenY(double worldY) const
    {
        // EasyX 的 Y 轴向下，所以世界相对摄像机中心的 Y 偏移需要取反。
        return (int)(WINDOW_HEIGHT / 2.0 - (worldY - centerY) * zoom);
    }

    // 功能：把世界空间尺寸转换为当前缩放下的屏幕尺寸。
    int worldSizeToScreen(double worldSize) const
    {
        // 用世界尺寸乘 zoom，得到屏幕像素尺寸。
        return (int)(worldSize * zoom);
    }

};


extern Camera gCamera;

int worldToScreenX(double worldX);
int worldToScreenY(double worldY);
int worldSizeToScreen(double worldSize);