#pragma once

#include <string>
#include "EntityManager.h"

struct Camera;

// 当前相机跟随目标 ID。
extern EntityID gCameraFollowTargetId;

// 功能：切换相机当前跟随的实体 ID。
void setCameraFollowTarget(EntityID newTargetId, const EntityManager& entityManager);

// 功能：根据跟随目标、鼠标偏移和缩放输入更新相机。
void updateCameraFollow(
    Camera& camera,
    EntityManager& entityManager,
    int worldWidth,
    int worldHeight,
    int mouseOffsetX,
    int mouseOffsetY
);
