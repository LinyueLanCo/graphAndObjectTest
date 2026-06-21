#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "Entity.h"
#include "AnimationClipManager.h"

// SpawnRequest: 动态生成实体的请求参数。
struct SpawnRequest
{
    double x;                // 诞生位置的世界坐标 X
    double y;                // 诞生位置的世界坐标 Y
    std::string templateName;// 实体模板名（如 "CoinSilver"）
};

// EntityTemplate: 实体模板定义，从 entity_templates.json 读取。
struct EntityTemplate
{
    std::string name;
    EntityType type = DEFAULT;
    bool controlled = false;
    bool collidable = true;
    bool blocking = false;
    bool god = false;
    double scaleX = 1.0;
    double scaleY = 1.0;
    double offsetX = 0.0;
    double offsetY = 0.0;
    double colScaleX = 1.0;
    double colScaleY = 1.0;
    double colOffsetX = 0.0;
    double colOffsetY = 0.0;
    int animSpeed = -1;
    std::string initialAnim = "idle";
    facingDirection initialFacing = RIGHT;
    double shakeDuration = 90.0;
    
    // std::unordered_map 是标准模板库（STL）中的哈希表关联容器。
    // 这里用于状态映射：将动画状态名称关联到实际的剪辑名称，提供常数平均时间复杂度的快速查询。
    std::unordered_map<std::string, std::string> stateToClip;

    // std::vector 是标准模板库（STL）中的动态数组容器，用于存储连续内存的元素。
    // 这里用于保存动画状态过渡规则表。
    std::vector<TransitionRule> transitionRules;
};

// EntityManager: 实体总管家，负责管理对象池和双索引。
class EntityManager
{
private:
    // entities: 对象池本尊。
    // std::vector 容器，用于固定存放 200 个实体实例的连续大箱子，避免运行时频繁分配与释放内存。
    std::vector<Entity> entities;

    // idToIndex: 导航地图。
    // std::unordered_map 关联容器，记录实体 ID 到大箱子下标的映射，支持常数时间复杂度的查找。
    std::unordered_map<EntityID, size_t> idToIndex;

    // spawnQueue: 临时寄存处。
    // std::vector 容器，保存本帧内请求动态生成但还没落地的演员队列，用于在帧末安全统一生成。
    std::vector<SpawnRequest> spawnQueue;

    EntityID nextEntityId;                               // 自增唯一标识符计数器

    // 双索引列表，高效遍历与复用的核心
    // activeIndices: 活跃索引名单。std::vector 容器，保存目前活在游戏世界里的实体在 entities 数组中的下标。
    // deadIndices: 空闲索引名单。std::vector 容器，保存死掉或者还没启用的槽位在 entities 数组中的下标。
    std::vector<size_t> activeIndices;
    std::vector<size_t> deadIndices;

    // templates: 实体模板库。
    // std::unordered_map 关联容器，将模板名映射到 EntityTemplate 结构体，用于快速生成实体。
    std::unordered_map<std::string, EntityTemplate> templates;

public:
    EntityManager();

    const std::vector<size_t>& getActiveIndices() const;

    // 从 JSON 配置文件载入模板库
    bool loadTemplates(const std::string& filepath);

    // 从 JSON 配置文件载入初始关卡的实体
    bool loadEntities(const std::string& filepath, AnimationClipManager& animationClips);

    // 把实时生成新实体的请求放入队列，等帧末安全处理
    void queueSpawnEntity(
        double x,
        double y,
        const std::string& templateName
    );

    // 帧末安全大扫除与新生实体的生成
    void processSpawns(AnimationClipManager& animationClips);

    // 两个极速 ID 导航函数，支持通过 EntityID 拿取实体的读写/只读指针（找不到就回 nullptr）
    Entity* getEntity(EntityID id);
    const Entity* getEntity(EntityID id) const;

    // 兼容老系统的 entities 数组引用接口，不推荐频繁遍历
    std::vector<Entity>& getEntities();
    const std::vector<Entity>& getEntities() const;

    // 清空大管家状态，清空所有队列和索引
    void clear();

    // 根据活跃名单，重新构建 nameToIndex 导航图
    void rebuildMap();

    void collectSprites(class RenderQueue& queue);
};
