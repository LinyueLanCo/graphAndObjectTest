#include "Collision.h"

// 功能：初始化碰撞盒的原始尺寸、偏移和缩放。
CollisionBox::CollisionBox()
{
    width = 0;
    height = 0;
    offsetX = 0.0;
    offsetY = 0.0;
    scaleX = 1.0;
    scaleY = 1.0;
}

// 功能：设置碰撞盒的原始尺寸。
void CollisionBox::setBaseSize(double newWidth, double newHeight)
{
    width = newWidth;
    height = newHeight;
}

// 功能：设置碰撞盒相对拥有者中心点的偏移。
void CollisionBox::setOffset(double newOffsetX, double newOffsetY)
{
    offsetX = newOffsetX;
    offsetY = newOffsetY;
}

// 功能：设置碰撞盒的缩放比例。
void CollisionBox::setScale(double newScaleX, double newScaleY)
{
    scaleX = newScaleX;
    scaleY = newScaleY;
}

// 功能：根据拥有者世界坐标生成最终用于检测的世界碰撞盒。
RectBox CollisionBox::toWorldBox(double ownerX, double ownerY)
{
    RectBox box;

    // 用拥有者中心点加碰撞盒偏移，得到碰撞盒自己的世界中心点。
    double colliderCenterX = ownerX + offsetX;
    double colliderCenterY = ownerY + offsetY;

    // 用原始宽高乘缩放，得到最终参与检测的世界宽高。
    double colliderWidth = width * scaleX;
    double colliderHeight = height * scaleY;

    // 用中心点加减半宽半高，得到 AABB 的四条世界边界。
    box.left = colliderCenterX - colliderWidth / 2.0;
    box.right = colliderCenterX + colliderWidth / 2.0;
    box.bottom = colliderCenterY - colliderHeight / 2.0;
    box.top = colliderCenterY + colliderHeight / 2.0;

    return box;
}
