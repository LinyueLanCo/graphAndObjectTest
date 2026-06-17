#pragma once

#include <string>
#include "EntityManager.h"

// 当前相机跟随目标 ID。
extern std::string gCameraFollowTargetId;

// 功能：切换相机当前跟随的实体 ID。
void setCameraFollowTarget(const std::string& newTargetId, const EntityManager& entityManager);

// 功能：根据跟随目标、鼠标偏移和缩放输入更新相机。
void updateCameraFollow(
    EntityManager& entityManager,
    int worldWidth,
    int worldHeight,
    int mouseOffsetX,
    int mouseOffsetY
);
