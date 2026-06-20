#pragma once

#include "Collision.h"
#include "Entity.h"
#include "TileMap.h"
#include "EntityManager.h"
#include <vector>
#include <string>

// CollisionManager
// 碰撞与物理判定管理系统，整合了原有的位移阻挡计算与实体的碰撞和重叠生命周期管理。
class CollisionManager
{
private:
    // std::vector 是标准模板库（STL）中的动态数组容器，用于存储连续内存的元素。
    // std::string 是标准模板库中的字符串类，代表字符序列。
    // 这里用于保存上一帧检测到的相交实体配对，如 "Player1_Coin1"，以防止每帧重复打印重叠日志。
    std::vector<std::string> lastOverlapPairs;

    // 辅助函数，用来判断指定的 key 字符串是否在 vec 向量容器中。
    // 接收 const 引用以避免拷贝开销。
    bool contains(const std::vector<std::string>& vec, const std::string& key);

public:
    // 计算两个轴对齐包围盒（AABB）矩形在二维平面上是否发生了重叠相交。
    // a 和 b 为矩形包围盒，包含上下左右四个边界的 double 值。
    bool isRectOverlapping(RectBox a, RectBox b);

    // 计算两个一维区间是否发生了相交。
    // aMin, aMax 为第一个区间的上下限，bMin, bMax 为第二个区间的上下限。
    bool isRangeOverlapping(
        double aMin,
        double aMax,
        double bMin,
        double bMax
    );

    // 计算实体在 X 轴（水平方向）的最大允许位移，返回物理修正后的安全移动像素值。
    // self: 当前正在计算移动的实体对象。
    // moveX: 本帧期望移动的水平偏移量。
    // entitys: std::vector 容器，保存所有实体的列表。
    // activeIndices: std::vector 容器，保存当前活跃实体在 entitys 中的下标，用来过滤死亡实体以优化性能。
    // selfIndex: 当前实体在 entitys 列表中的槽位索引，用于在遍历时跳过自身。
    // tileMap: 瓦片地图引用，用于查询地图上的固体图块阻挡情况。
    double getAllowedMoveX(
        Entity& self,
        double moveX,
        std::vector<Entity>& entitys,
        const std::vector<size_t>& activeIndices,
        int selfIndex,
        TileMap& tileMap
    );

    // 计算实体在 Y 轴（垂直方向）的最大允许位移，返回物理修正后的安全移动像素值。
    // 参数含义同上，计算时要考虑单向平台在下坠时的特殊碰撞规则。
    double getAllowedMoveY(
        Entity& self,
        double moveY,
        std::vector<Entity>& entitys,
        const std::vector<size_t>& activeIndices,
        int selfIndex,
        TileMap& tileMap
    );

    // 世界边界限制。
    // 强行修正实体的物理位置（x，y 坐标），使其无法超出世界总宽度 and 高度，并对碰壁/触地的物理状态（如 blockedByWorld、onGround、velocityY）进行修正。
    void limitInWorld(
        Entity& self,
        int worldWidth,
        int worldHeight
    );

    // 统一检测所有活跃实体间的两两碰撞并填充碰撞列表。
    // 内部进行两两包围盒相交测试，并更新实体对象内的重叠链表。
    void updateOverlapEvents(EntityManager& entityManager);

    // 统一处理实体碰撞事件，让实体各自调用 resolveOverlaps 做出特定的碰撞反应（如收集金币或死亡）。
    void resolveEntityOverlaps(EntityManager& entityManager);

    // 清理碰撞历史记录（通常在关卡初始化时调用，避免上一关的残留状态干扰新关卡）。
    void clearHistory();
};
