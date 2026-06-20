#pragma once

#include <string>

// EntityType：
// 当前简易实体分类，用于区分玩家、普通实体、金币等基础逻辑分支。
enum EntityType
{
    PLAYER = 1,
    ENTITY = 2,
    COIN = 3,
    DEFAULT = 4,
    CHECKPOINT = 5,
    ENDPOINT = 6,
    FRUIT = 7
};

// facingDirection：
// 记录实体最后一次有效朝向，没有移动输入时用于决定待机动画方向。
enum facingDirection
{
    LEFT,
    RIGHT,
    UP,
    DOWN
};

using EntityID = int;
const EntityID INVALID_ENTITY_ID = 0;

// OverlapInfo：
// 记录发生碰撞重叠的对方实体 ID 和实体类型。
struct OverlapInfo
{
    EntityID otherEntityId;
    EntityType otherType;
};



