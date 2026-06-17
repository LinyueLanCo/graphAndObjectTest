#include "BackgroundObject.h"
#include "Camera.h"

BackgroundObject::BackgroundObject()
{
    renderOrder = 0;

    parallaxFactor = 1.0;
    zoomFactor = 0.0;

    visible = true;
    useAlphaBlend = true;
    generatedByTiling = false;

    drawMode = BACKGROUND_SINGLE_WORLD;

    centerX = 0.0;
    centerY = 0.0;
    runtimeCenterX = 0.0;
    runtimeCenterY = 0.0;
    drawW = 0.0;
    drawH = 0.0;
}

// 功能：把背景对象的 sprite 绑定到指定图片资源。
void BackgroundObject::bindSpriteSource(Image2D* imageSource)
{
    if (imageSource == NULL)
    {
        renderSprite.visible = false;
        return;
    }

    int imageW = imageSource->getWidth();
    int imageH = imageSource->getHeight();

    if (imageW <= 0 || imageH <= 0)
    {
        renderSprite.visible = false;
        return;
    }

    renderSprite.setSource(
        imageSource,
        0,
        0,
        imageW,
        imageH
    );
}

// 功能：设置背景对象的渲染顺序、视差参数和绘制模式。
void BackgroundObject::setRenderData(
    int newRenderOrder,
    double newParallaxFactor,
    double newZoomFactor,
    bool newUseAlphaBlend,
    BackgroundDrawMode newDrawMode
)
{
    renderOrder = newRenderOrder;
    parallaxFactor = newParallaxFactor;
    zoomFactor = newZoomFactor;
    useAlphaBlend = newUseAlphaBlend;
    drawMode = newDrawMode;
}

// 功能：设置背景对象的基础世界中心点和世界绘制尺寸。
void BackgroundObject::setDrawData(double newCenterX, double newCenterY, double newDrawW, double newDrawH)
{
    centerX = newCenterX;
    centerY = newCenterY;
    runtimeCenterX = centerX;
    runtimeCenterY = centerY;
    drawW = newDrawW;
    drawH = newDrawH;
}

// 功能：把背景对象当前运行时位置同步到自己的 sprite。
void BackgroundObject::updateSprite()
{
    renderSprite.visible = visible;

    if (!visible)
    {
        return;
    }

    double finalDrawW = drawW;
    double finalDrawH = drawH;

    if (finalDrawW <= 0)
    {
        finalDrawW = renderSprite.srcW;
    }

    if (finalDrawH <= 0)
    {
        finalDrawH = renderSprite.srcH;
    }

    renderSprite.setWorldDrawData(
        runtimeCenterX,
        runtimeCenterY,
        finalDrawW,
        finalDrawH
    );
}

// 功能：根据背景模式更新本帧用于绘制的运行时逻辑中心点。
void BackgroundObject::updateRuntimeTransform(double parallaxOffsetX, double parallaxOffsetY)
{
    if (drawMode == BACKGROUND_FIXED_CAMERA)
    {
        // fixed 背景把运行时中心锁到 Camera 中心，使背景看起来固定在视口里。
        runtimeCenterX = gCamera.centerX;
        runtimeCenterY = gCamera.centerY;
        return;
    }

    if (drawMode == BACKGROUND_SINGLE_WORLD)
    {
        // 普通世界背景不额外处理视差，直接使用对象自己的基础逻辑位置。
        runtimeCenterX = centerX;
        runtimeCenterY = centerY;
        return;
    }

    if (drawMode == BACKGROUND_REPEAT_X)
    {
        // parallaxFactor 表示背景在屏幕上相对地图的移动比例。
        // 0.0 接近固定在屏幕上，1.0 接近普通世界物体。
        runtimeCenterX = centerX + parallaxOffsetX * (1.0 - parallaxFactor);
        runtimeCenterY = centerY + parallaxOffsetY * (1.0 - parallaxFactor);
        return;
    }

    runtimeCenterX = centerX;
    runtimeCenterY = centerY;
}
