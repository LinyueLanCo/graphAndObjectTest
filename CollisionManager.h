#pragma once

#include "Collision.h"
#include "Entity.h"
#include "TileMap.h"
#include "EntityManager.h"
#include <vector>
#include <string>

// CollisionManager:
// 碰撞与物理判定管理系统，整合了原有的位移阻挡计算与实体的碰撞/重叠生命周期管理。
class CollisionManager
{
private:
    std::vector<std::string> lastOverlapPairs;

    bool contains(const std::vector<std::string>& vec, const std::string& key);

public:
    // 计算两个 AABB 矩形在二维平面上是否发生了重叠相交
    bool isRectOverlapping(RectBox a, RectBox b);

    // 计算两个一维区间是否发生了相交
    bool isRangeOverlapping(
        double aMin,
        double aMax,
        double bMin,
        double bMax
    );

    // 计算实体在 X 轴（水平方向）的最大允许位移，返回物理修正后的安全移动像素值
    double getAllowedMoveX(
        Entity& self,
        double moveX,
        std::vector<Entity>& entitys,
        const std::vector<size_t>& activeIndices,
        int selfIndex,
        TileMap& tileMap
    );

    // 计算实体在 Y 轴（垂直方向）的最大允许位移，返回物理修正后的安全移动像素值
    double getAllowedMoveY(
        Entity& self,
        double moveY,
        std::vector<Entity>& entitys,
        const std::vector<size_t>& activeIndices,
        int selfIndex,
        TileMap& tileMap
    );

    // 世界边界限制：强行把实体锁在关卡屏幕边缘内，不让它掉到关卡下方或者走到地图左边界外面
    void limitInWorld(
        Entity& self,
        int worldWidth,
        int worldHeight
    );

    // 统一检测所有活跃实体间的两两碰撞并填充碰撞列表
    void updateOverlapEvents(EntityManager& entityManager);

    // 统一处理实体碰撞事件，让实体自治响应
    void resolveEntityOverlaps(EntityManager& entityManager);

    // 清理碰撞历史记录（关卡初始化时调用）
    void clearHistory();
};
