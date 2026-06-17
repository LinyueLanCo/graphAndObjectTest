#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "Entity.h"
#include "AnimationClipManager.h"

struct SpawnRequest
{
    std::string id;
    double x;
    double y;
    bool controlled;
    bool collidable;
    bool blocking;
    bool god;
    EntityType type;
    AnimationSetId animSet;
    double scaleX;
    double scaleY;
    double colScaleX;
    double colScaleY;
    int animSpeed;
};

class EntityManager
{
private:
    std::vector<Entity> entities;
    std::unordered_map<std::string, size_t> nameToIndex;
    std::vector<SpawnRequest> spawnQueue;

public:
    // 缓存重叠历史，防止重复打印（取代 Level 的 2D vector 缓存）
    std::unordered_set<std::string> lastOverlapPairs;

    EntityManager();

    // 从 JSON 配置文件加载所有实体
    bool loadEntities(const std::string& filepath, AnimationClipManager& animationClips);

    // 实时动态生成实体的请求入队
    void queueSpawnEntity(
        const std::string& id,
        double x,
        double y,
        bool controlled,
        bool collidable,
        bool blocking,
        bool god,
        EntityType type,
        AnimationSetId animSet,
        double scaleX = 1.0,
        double scaleY = 1.0,
        double colScaleX = 1.0,
        double colScaleY = 1.0,
        int animSpeed = -1
    );

    // 帧末安全处理生成列表，避免迭代器失效
    void processSpawns(AnimationClipManager& animationClips);

    // 根据实体 ID 获取实体指针
    Entity* getEntityById(const std::string& id);
    const Entity* getEntityById(const std::string& id) const;

    // 获取实体数组的只读/读写引用（以便兼容已有系统）
    std::vector<Entity>& getEntities();
    const std::vector<Entity>& getEntities() const;

    // 清空管理器
    void clear();

    // 重建 ID 映射表
    void rebuildMap();
};
