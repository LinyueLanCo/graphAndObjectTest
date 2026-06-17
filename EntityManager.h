#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "Entity.h"
#include "AnimationClipManager.h"

class EntityManager
{
private:
    std::vector<Entity> entities;
    std::unordered_map<std::string, size_t> nameToIndex;

public:
    // 缓存重叠历史，防止重复打印（取代 Level 的 2D vector 缓存）
    std::unordered_set<std::string> lastOverlapPairs;

    EntityManager();

    // 从 JSON 配置文件加载所有实体
    bool loadEntities(const std::string& filepath, AnimationClipManager& animationClips);

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
