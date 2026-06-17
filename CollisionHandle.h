#pragma once

#include "Collision.h"
#include "Entity.h"
#include "TileMap.h"

// CollisionHandle: 物理碰撞盒判定工具包，专门用作移动时的“阻挡刹车系统”。
// 主要负责：
// 1. 判断两个 AABB 矩形或一维区间是否重叠。
// 2. 根据实体的期望位移，计算其与瓦片地图、其它阻挡物之间的允许移动距离，防止下一帧卡进墙里。
class CollisionHandle
{
public:
    // 计算两个 AABB 矩形（a 和 b）在二维平面上是否发生了重叠相交
    bool isRectOverlapping(RectBox a, RectBox b);

    // 计算两个一维区间是否发生了相交。比如在判断垂直撞头时，需要先测 X 轴投影范围是否重叠
    // 参数意义：
    //   aMin, aMax: 区间 A 的左右/上下端点
    //   bMin, bMax: 区间 B 的左右/上下端点
    bool isRangeOverlapping(
        double aMin,
        double aMax,
        double bMin,
        double bMax
    );

    // 计算实体在 X 轴（水平方向）的最大允许位移，返回物理修正后的安全移动像素值
    // 参数意义：
    //   self: 当前正在更新移动的角色本体
    //   moveX: 本帧键盘或物理期望它水平移动的位移（正数往右，负数往左）
    //   entitys: 实体池，用来检索其它实体
    //   activeIndices: 活人名单，仅对活着的阻挡物做碰撞
    //   selfIndex: 角色自己在对象池的槽位号（为了排除自己和自己撞的乌龙）
    //   tileMap: 瓦片格地图，计算它是否撞了格子墙体
    double getAllowedMoveX(
        Entity& self,
        double moveX,
        std::vector<Entity>& entitys,
        const std::vector<size_t>& activeIndices,
        int selfIndex,
        TileMap& tileMap
    );

    // 计算实体在 Y 轴（垂直方向）的最大允许位移，返回物理修正后的安全移动像素值
    // 参数意义：
    //   self: 当前正在更新移动的角色本体
    //   moveY: 本帧重力下落或跳跃冲劲期望它垂直移动的距离（正数往上，负数往下）
    //   entitys: 实体池
    //   activeIndices: 活跃实体名单，仅对活着的实体做计算
    //   selfIndex: 自己在对象池里的槽位号，用作自碰撞排除
    //   tileMap: 瓦片格地图，用来算它是否落地、撞天花板或单向平台
    double getAllowedMoveY(
        Entity& self,
        double moveY,
        std::vector<Entity>& entitys,
        const std::vector<size_t>& activeIndices,
        int selfIndex,
        TileMap& tileMap
    );

    // 世界边界限制：强行把实体锁在关卡屏幕边缘内，不让它掉到关卡下方或者走到地图左边界外面
    // 参数意义：
    //   self: 待检测限制的实体
    //   worldWidth: 世界最大宽度（单位像素）
    //   worldHeight: 世界最大高度（单位像素）
    void limitInWorld(
        Entity& self,
        int worldWidth,
        int worldHeight
    );
};
