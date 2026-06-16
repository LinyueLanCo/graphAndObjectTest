#pragma once

#include "Entity.h"

// 当前相机跟随目标下标。
// 这是一个临时全局变量，后续可以继续迁移到 CameraHandle 或 Level 内部。
extern int gCameraFollowTargetIndex;

// 功能：切换相机当前跟随的实体下标。
void setCameraFollowTarget(int newTargetIndex, vector<Entity>& entitys);

// 功能：根据跟随目标、鼠标偏移和缩放输入更新相机。
void updateCameraFollow(
    vector<Entity>& entitys,
    int worldWidth,
    int worldHeight,
    int mouseOffsetX,
    int mouseOffsetY
);
