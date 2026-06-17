#include "EntityManager.h"
#include <fstream>
#include <iostream>
#include "json.hpp"

EntityManager::EntityManager()
{
}

bool EntityManager::loadEntities(const std::string& filepath, AnimationClipManager& animationClips)
{
    std::ifstream f(filepath);
    if (!f.is_open())
    {
        std::cout << "Failed to open entities config: " << filepath << std::endl;
        return false;
    }

    nlohmann::json data;
    f >> data;
    f.close();

    entities.clear();
    nameToIndex.clear();

    // 预留空间，避免 vector 频繁扩容
    entities.reserve(data.size() + 5);

    for (auto& item : data)
    {
        std::string name = item.value("id", "");
        double x = item.value("x", 0.0);
        double y = item.value("y", 0.0);
        bool controlled = item.value("controlled", false);
        bool collidable = item.value("collidable", true);
        bool blocking = item.value("blocking", false);
        bool god = item.value("god", false);
        EntityType type = (EntityType)item.value("type", 4);
        AnimationSetId animSet = (AnimationSetId)item.value("animSet", 0);

        entities.emplace_back(
            name,
            x, y,
            controlled, collidable, blocking, god,
            type, animSet,
            1 // alive = 1
        );

        Entity& ent = entities.back();

        // 读取可选的 JSON 属性并应用到实体上
        double scaleX = item.value("scaleX", 1.0);
        double scaleY = item.value("scaleY", 1.0);
        double offsetX = item.value("offsetX", 0.0);
        double offsetY = item.value("offsetY", 0.0);
        ent.setSpriteTransform(scaleX, scaleY, offsetX, offsetY);

        double colScaleX = item.value("colScaleX", 1.0);
        double colScaleY = item.value("colScaleY", 1.0);
        ent.setCollisionScale(colScaleX, colScaleY);

        double colOffsetX = item.value("colOffsetX", 0.0);
        double colOffsetY = item.value("colOffsetY", 0.0);
        ent.setCollisionBoxOffset(colOffsetX, colOffsetY);

        int animSpeed = item.value("animSpeed", -1);
        if (animSpeed != -1)
        {
            ent.setAnimationSpeed(animSpeed);
        }
    }

    // 为所有加载完成的实体同步初始化动画帧
    for (auto& ent : entities)
    {
        ent.initAnimationFromAnimator(animationClips);
    }

    rebuildMap();
    return true;
}

Entity* EntityManager::getEntityById(const std::string& id)
{
    auto it = nameToIndex.find(id);
    if (it != nameToIndex.end())
    {
        // 保证返回有效的指针，且底层 entities 发生扩容前此指针在当前帧有效
        return &entities[it->second];
    }
    return nullptr;
}

const Entity* EntityManager::getEntityById(const std::string& id) const
{
    auto it = nameToIndex.find(id);
    if (it != nameToIndex.end())
    {
        return &entities[it->second];
    }
    return nullptr;
}

std::vector<Entity>& EntityManager::getEntities()
{
    return entities;
}

const std::vector<Entity>& EntityManager::getEntities() const
{
    return entities;
}

void EntityManager::clear()
{
    entities.clear();
    nameToIndex.clear();
    spawnQueue.clear();
    lastOverlapPairs.clear();
}

void EntityManager::rebuildMap()
{
    nameToIndex.clear();
    for (size_t i = 0; i < entities.size(); ++i)
    {
        nameToIndex[entities[i].getId()] = i;
    }
}

void EntityManager::queueSpawnEntity(
    const std::string& id,
    double x,
    double y,
    bool controlled,
    bool collidable,
    bool blocking,
    bool god,
    EntityType type,
    AnimationSetId animSet,
    double scaleX,
    double scaleY,
    double colScaleX,
    double colScaleY,
    int animSpeed
)
{
    SpawnRequest req;
    req.id = id;
    req.x = x;
    req.y = y;
    req.controlled = controlled;
    req.collidable = collidable;
    req.blocking = blocking;
    req.god = god;
    req.type = type;
    req.animSet = animSet;
    req.scaleX = scaleX;
    req.scaleY = scaleY;
    req.colScaleX = colScaleX;
    req.colScaleY = colScaleY;
    req.animSpeed = animSpeed;

    spawnQueue.push_back(req);
}

void EntityManager::processSpawns(AnimationClipManager& animationClips)
{
    if (spawnQueue.empty())
    {
        return;
    }

    for (const auto& req : spawnQueue)
    {
        entities.emplace_back(
            req.id,
            req.x,
            req.y,
            req.controlled,
            req.collidable,
            req.blocking,
            req.god,
            req.type,
            req.animSet,
            1 // alive = 1
        );

        Entity& ent = entities.back();
        ent.setSpriteTransform(req.scaleX, req.scaleY, 0.0, 0.0);
        ent.setCollisionScale(req.colScaleX, req.colScaleY);

        if (req.animSpeed != -1)
        {
            ent.setAnimationSpeed(req.animSpeed);
        }

        // 初始化动画
        ent.initAnimationFromAnimator(animationClips);

        std::cout << "Dynamic Spawn: Entity ID \"" << req.id << "\" spawned at (" << req.x << ", " << req.y << ")." << std::endl;
    }

    spawnQueue.clear();
    rebuildMap();
}
