#pragma once

#include "Collision.h"
#include "Entity.h"
#include "TileMap.h"

// CollisionHandle：
// 碰撞底层能力提供者。
// 主要职责：
//   1. 判断两个 AABB 是否重叠：isRectOverlapping
//   2. 判断一维范围是否重叠：isRangeOverlapping
//   3. 根据期望位移计算允许位移：getAllowedMoveX/Y
//   4. 把实体限制在世界边界内：limitInWorld
//
// 阻挡碰撞和重叠事件的底层都和 AABB / overlap 有关，
// 但是用途不同：
//   - 重叠事件：关心“两个盒子现在有没有重叠”
//   - 阻挡移动：关心“想移动这么多，最多允许移动多少”
//
// 当前 getAllowedMoveX/Y 并不是简单地“把实体移动到下一帧再判断是否 overlap”，
// 而是用当前碰撞盒 + 移动方向 + 另一轴范围重叠，计算离最近阻挡物还有多远。
// 它本质上是在“阻止下一帧发生 overlap”。
class CollisionHandle
{
public:
    // 功能：声明两个 AABB 矩形重叠检测接口。
    bool isRectOverlapping(RectBox a, RectBox b);

    // 功能：声明两个一维区间重叠检测接口。
    bool isRangeOverlapping(
        double aMin,
        double aMax,
        double bMin,
        double bMax
    );

    // 功能：声明 X 轴允许位移计算接口。
    double getAllowedMoveX(
        Entity& self,
        double moveX,
        vector<Entity>& entitys,
        int selfIndex,
        TileMap& tileMap
    );

    // 功能：声明 Y 轴允许位移计算接口。
    double getAllowedMoveY(
        Entity& self,
        double moveY,
        vector<Entity>& entitys,
        int selfIndex,
        TileMap& tileMap
    );

    // 功能：声明实体世界边界限制接口。
    void limitInWorld(
        Entity& self,
        int worldWidth,
        int worldHeight
    );
};
