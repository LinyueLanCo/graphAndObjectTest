#pragma once

#include "BackgroundTypes.h"
#include "Resource.h"
#include "Sprite.h"

// BackgroundObject:
// 背景层中的一个具体对象。它保存自己的逻辑位置、视差规则和最终交给 Renderer 的 sprite。
struct BackgroundObject
{
    double zoomFactor;
    BackgroundDrawMode drawMode;

    int renderOrder;
    double parallaxFactor;
    bool visible;
    bool useAlphaBlend;
    bool generatedByTiling;

    double centerX;
    double centerY;
    double runtimeCenterX;
    double runtimeCenterY;
    double drawW;
    double drawH;
    double vx;
    double vy;

    sprite renderSprite;

    BackgroundObject();

    void bindSpriteSource(Image2D* imageSource);
    void setRenderData(
        int newRenderOrder,
        double newParallaxFactor,
        double newZoomFactor,
        bool newUseAlphaBlend,
        BackgroundDrawMode newDrawMode
    );
    void setDrawData(double newCenterX, double newCenterY, double newDrawW, double newDrawH);
    void updateSprite();
    void updateRuntimeTransform(double cameraVx, double cameraVy);
};
