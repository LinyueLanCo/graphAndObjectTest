#pragma once

#include "CollisionHandle.h"
#include "Controller.h"
#include "Entity.h"
#include "TileMap.h"

// MovementHandle：
// 移动/物理执行系统。
// 它读取 BehaviorIntent，计算速度、冲刺、跳跃、重力和期望位移。
// 它不亲自判断碰撞细节，而是把 wantMoveX/wantMoveY 交给 CollisionHandle，
// 得到 allowedMoveX/allowedMoveY 后，再把结果写回 Entity。
//
// 重要公式：
//   currentSpeed = speed 或 speed * 2
//   wantMoveX = inputX * currentSpeed
//   velocityY -= GRAVITY
//   wantMoveY = velocityY
//   actualMove = allowedMove
class MovementHandle
{
public:
    // 功能：声明实体移动与物理更新接口。
    void update(
        Entity& self,
        BehaviorIntent intent,
        vector<Entity>& entitys,
        int selfIndex,
        TileMap& tileMap,
        int worldWidth,
        int worldHeight,
        CollisionHandle& collisionHandle
    );
};
