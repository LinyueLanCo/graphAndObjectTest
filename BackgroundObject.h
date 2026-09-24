#pragma once

#include "BackgroundTypes.h"
#include "Resource.h"
#include "Sprite.h"
#include "Vector2D.h"

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

    Vector2D center;
    Vector2D runtimeCenter;
    Vector2D velocity;

    double drawW;
    double drawH;
    double autoScrollSpeedX;

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

    void setDrawData(
        const Vector2D& newCenter,
        double newDrawW,
        double newDrawH
    );

    // 兼容旧的 x/y 调用形式。
    void setDrawData(
        double newCenterX,
        double newCenterY,
        double newDrawW,
        double newDrawH
    )
    {
        setDrawData(
            Vector2D(newCenterX, newCenterY),
            newDrawW,
            newDrawH
        );
    }

    void updateSprite();

    void updateRuntimeTransform(const Vector2D& parallaxCamera);
};
