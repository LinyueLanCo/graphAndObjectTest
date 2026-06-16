#pragma once

#include "Resource.h"

// sprite：
 // 单帧渲染数据容器。
 // 它只记录当前要绘制的图片来源、源图裁剪矩形、可见性、缩放和偏移。
 // 它不负责动画播放、不负责动画状态切换，也不直接调用 EasyX 绘制函数。
struct sprite
{
    Image2D* imageSource;

    int srcX;
    int srcY;
    int srcW;
    int srcH;

    bool visible;

    double scaleX;
    double scaleY;
    double offsetX;
    double offsetY;

    double worldCenterX;
    double worldCenterY;

    double worldDrawW;
    double worldDrawH;

    // 功能：初始化一个空精灵，默认没有图、帧矩形和变换。
    sprite() : imageSource(NULL),
        srcX(0),
        srcY(0),
        srcW(0),
        srcH(0),
        scaleX(1.0),
        scaleY(1.0),
        offsetX(0.0),
        offsetY(0.0),
        worldCenterX(0.0),
        worldCenterY(0.0),
        worldDrawW(0.0),
        worldDrawH(0.0),
        visible(true)
    {}
    // 功能：设置精灵当前帧使用的图像资源和源图裁剪矩形。
    void setSource(Image2D* newImageSource, int newSrcX, int newSrcY, int newSrcW, int newSrcH)
    {
        imageSource = newImageSource;
        srcX = newSrcX;
        srcY = newSrcY;
        srcW = newSrcW;
        srcH = newSrcH;
    }

    // 功能：设置精灵的缩放和相对实体中心的偏移。
    void setTransform(double newScaleX, double newScaleY, double newOffsetX, double newOffsetY)
    {
        scaleX = newScaleX;
        scaleY = newScaleY;
        offsetX = newOffsetX;
        offsetY = newOffsetY;
    }

    // 功能：设置 sprite 在世界坐标中的最终绘制中心点和绘制尺寸。
    void setWorldDrawData(double newCenterX, double newCenterY, double newDrawW, double newDrawH)
    {
        worldCenterX = newCenterX;
        worldCenterY = newCenterY;
        worldDrawW = newDrawW;
        worldDrawH = newDrawH;
    }
};
