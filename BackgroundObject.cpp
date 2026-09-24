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
    vx = 0.0;
    vy = 0.0;
    autoScrollSpeedX = 0.0;
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

// 功能：根据背景模式和“视差相机参考位置”直接计算本帧运行时中心点。
// 注意这里接收的是绝对的 Parallax Camera Position，不再是逐帧 delta。
// 因此 runtimeCenter 不会因为某一帧错误的 Camera Delta 被永久污染。
void BackgroundObject::updateRuntimeTransform(double parallaxCameraX, double parallaxCameraY)
{
    double oldRuntimeX = runtimeCenterX;
    double oldRuntimeY = runtimeCenterY;

    centerX += autoScrollSpeedX;

    if (drawMode == BACKGROUND_FIXED_CAMERA)
    {
        // fixed 背景直接锁到最终 View Camera。
        runtimeCenterX = gCamera.centerX;
        runtimeCenterY = gCamera.centerY;
    }
    else if (drawMode == BACKGROUND_SINGLE_WORLD)
    {
        // 普通世界背景保持自己的世界位置。
        runtimeCenterX = centerX;
        runtimeCenterY = centerY;
    }
    else if (drawMode == BACKGROUND_REPEAT_X)
    {
        // parallaxFactor 表示该层相对普通世界的视差倍率。
        // 0.0：接近固定在屏幕；1.0：普通世界；>1.0：前景反向加强。
        runtimeCenterX =
            centerX + parallaxCameraX * (1.0 - parallaxFactor);

        runtimeCenterY =
            centerY + parallaxCameraY * (1.0 - parallaxFactor);
    }
    else
    {
        runtimeCenterX = centerX;
        runtimeCenterY = centerY;
    }

    vx = runtimeCenterX - oldRuntimeX;
    vy = runtimeCenterY - oldRuntimeY;
}
