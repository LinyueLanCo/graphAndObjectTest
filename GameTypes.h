#pragma once

// EntityType：
// 当前简易实体分类，用于区分玩家、普通实体、金币等基础逻辑分支。
enum EntityType
{
    PLAYER = 1,
    ENTITY = 2,
    COIN = 3,
    DEFAULT = 4
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

// OverlapInfo：
// 记录与当前实体发生重叠的另一个实体的信息。
struct OverlapInfo
{
    int otherEntityId;
    EntityType otherType;
};

