#pragma once

// RectBox:
// 世界坐标下真正用于检测的 AABB 矩形盒。
// left/right/bottom/top 分别表示四条边。
struct RectBox
{
    double left;
    double right;
    double bottom;
    double top;
};

// CollisionBox:
// Entity 本地碰撞盒配置。它保存原始尺寸、偏移和缩放，并可换算成世界碰撞盒。
struct CollisionBox
{
    double width;
    double height;
    double offsetX;
    double offsetY;
    double scaleX;
    double scaleY;

    CollisionBox();

    void setBaseSize(double newWidth, double newHeight);
    void setOffset(double newOffsetX, double newOffsetY);
    void setScale(double newScaleX, double newScaleY);
    RectBox toWorldBox(double ownerX, double ownerY);
};
