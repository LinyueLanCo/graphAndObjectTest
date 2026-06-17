#pragma once

#include "CollisionHandle.h"
#include "Controller.h"
#include "Entity.h"
#include "TileMap.h"

// MovementHandle: 物理移动处理器。
// 职责包括：
// 1. 读取角色本帧的移动意图（水平走/跑、跳跃等）。
// 2. 累加重力更新纵向速度，并计算期望的 X/Y 位移。
// 3. 把位移丢给 CollisionHandle 进行碰撞阻挡计算，获得最终允许移动的实际位移。
// 4. 更新实体坐标，并标记落地、碰壁等物理状态。
class MovementHandle
{
public:
    // 计算并更新单个实体的物理移动位置
    // 参数意义：
    //   self: 当前待物理更新更新的实体引用
    //   intent: 本帧该实体的行为意图（想往哪移、想不想跳、想不想跑）
    //   entitys: 实体池，用来进行实体与实体间的碰撞判定
    //   activeIndices: 活跃实体索引表，用来过滤多余的死人槽位计算
    //   selfIndex: 自己在对象池里的下标槽位，用来做自碰撞排除
    //   tileMap: 瓦片格地图，用来算它是否撞了格子墙体
    //   worldWidth: 关卡世界的像素最大宽度
    //   worldHeight: 关卡世界的像素最大高度
    //   collisionHandle: 碰撞盒子判定处理器，具体测算障碍物距离
    void update(
        Entity& self,
        BehaviorIntent intent,
        vector<Entity>& entitys,
        const vector<size_t>& activeIndices,
        int selfIndex,
        TileMap& tileMap,
        int worldWidth,
        int worldHeight,
        CollisionHandle& collisionHandle
    );
};
