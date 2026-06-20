#include "EntityManager.h"
#include <fstream>
#include <iostream>
// 引入 nlohmann/json 第三方模板库。它通过非常高级的 C++ 模板元编程技术，
// 让我们能够像操作 Python 字典/JS 对象一样直接解析、读取和写入 JSON 配置文件。
// 我们在初始化关卡实体时，需要用它来解析 assets/data/entities.json 文件。
#include "json.hpp"
#include "RenderQueue.h"

// 默认构造，暂时不需要做什么
EntityManager::EntityManager()
{
}

// 获取活跃实体索引名单，外部系统（如 Level 和 Renderer）都是通过它来做无空转遍历的
const std::vector<size_t>& EntityManager::getActiveIndices() const
{
    return activeIndices;
}

// 从 JSON 配置文件中载入实体模板库
bool EntityManager::loadTemplates(const std::string& filepath)
{
    std::ifstream f(filepath);
    if (!f.is_open())
    {
        std::cout << "无法打开实体模板配置文件: " << filepath << std::endl;
        return false;
    }

    nlohmann::json data;
    try {
        f >> data;
        f.close();
    }
    catch (const std::exception& e) {
        std::cout << "解析实体模板 JSON 出错: " << e.what() << std::endl;
        f.close();
        return false;
    }

    templates.clear();

    for (auto& element : data.items())
    {
        std::string tempName = element.key();
        auto& config = element.value();

        EntityTemplate temp;
        temp.name = tempName;
        temp.type = (EntityType)config.value("type", 4);
        temp.controlled = config.value("controlled", false);
        temp.collidable = config.value("collidable", true);
        temp.blocking = config.value("blocking", false);
        temp.god = config.value("god", false);
        temp.scaleX = config.value("scaleX", 1.0);
        temp.scaleY = config.value("scaleY", 1.0);
        temp.offsetX = config.value("offsetX", 0.0);
        temp.offsetY = config.value("offsetY", 0.0);
        temp.colScaleX = config.value("colScaleX", 1.0);
        temp.colScaleY = config.value("colScaleY", 1.0);
        temp.colOffsetX = config.value("colOffsetX", 0.0);
        temp.colOffsetY = config.value("colOffsetY", 0.0);
        temp.animSpeed = config.value("animSpeed", -1);
        temp.initialAnim = config.value("initialAnim", "idle");

        std::string facingStr = config.value("facing", "RIGHT");
        if (facingStr == "LEFT") temp.initialFacing = LEFT;
        else if (facingStr == "RIGHT") temp.initialFacing = RIGHT;
        else if (facingStr == "UP") temp.initialFacing = UP;
        else if (facingStr == "DOWN") temp.initialFacing = DOWN;

        if (config.contains("animations"))
        {
            for (auto& animPair : config["animations"].items())
            {
                temp.stateToClip[animPair.key()] = animPair.value().get<std::string>();
            }
        }

        templates[tempName] = temp;
    }

    std::cout << "实体模板加载完毕，共加载了 " << templates.size() << " 个模板。" << std::endl;
    return true;
}

// 核心初始化：从 JSON 关卡配置文件中加载所有关卡实体，并分配固定对象池
bool EntityManager::loadEntities(const std::string& filepath, AnimationClipManager& animationClips)
{
    std::ifstream f(filepath);
    if (!f.is_open())
    {
        std::cout << "哎呀，打不开关卡配置文件: " << filepath << std::endl;
        return false;
    }

    nlohmann::json data;
    f >> data;
    f.close();

    // 1. 重要：初始化分配 200 个固定槽位，使得 vector 的大容量在游戏运行期不再变动。
    //    这样 entities 内 the Entity 实例物理内存地址就固定了，指针永远安全！
    entities.clear();
    entities.resize(200);

    // 清理索引、队列和哈希定位器，准备重新分配
    activeIndices.clear();
    deadIndices.clear();
    idToIndex.clear();
    spawnQueue.clear();
    nextEntityId = 1;

    size_t activeCount = data.size();
    if (activeCount > 200)
    {
        std::cout << "警告！JSON 里的实体数量超过了对象池上限 200 个，只能截取前 200 个。" << std::endl;
        activeCount = 200;
    }

    // 2. 载入配置文件里的第一批活跃实体
    for (size_t i = 0; i < activeCount; i++)
    {
        auto& item = data[i];
        std::string tempName = item.value("template", "");
        std::string name = item.value("name", tempName);
        double x = item.value("x", 0.0);
        double y = item.value("y", 0.0);

        auto it = templates.find(tempName);
        if (it == templates.end())
        {
            std::cout << "警告：实体 \"" << name << "\" 指定了未置的模板 \"" << tempName << "\"，无法加载！" << std::endl;
            continue;
        }
        const EntityTemplate& temp = it->second;

        // 如果 JSON 实例级别显式声明了覆盖参数，则用实例参数；否则使用模板默认参数
        bool controlled = item.value("controlled", temp.controlled);
        bool collidable = item.value("collidable", temp.collidable);
        bool blocking = item.value("blocking", temp.blocking);
        bool god = item.value("god", temp.god);
        EntityType type = (EntityType)item.value("type", (int)temp.type);

        EntityID iid = nextEntityId++;

        // 用 reset 函数重写这块槽位上实体的所有属性，“赋予新生命”
        entities[i].reset(
            iid,
            name,
            x, y,
            controlled, collidable, blocking, god,
            type, tempName,
            true // 初始活泼状态为 1（alive = 1）
        );

        // 同步加载并本地缓存实体的所有状态动画片段，重置朝向和初始动画状态
        entities[i].initAnimations(
            temp.name,
            temp.initialAnim,
            temp.initialFacing,
            temp.stateToClip,
            animationClips
        );

        // 读取 JSON 配置中的渲染参数（有覆盖用覆盖，无则用模板）
        double scaleX = item.value("scaleX", temp.scaleX);
        double scaleY = item.value("scaleY", temp.scaleY);
        double offsetX = item.value("offsetX", temp.offsetX);
        double offsetY = item.value("offsetY", temp.offsetY);
        entities[i].setSpriteTransform(scaleX, scaleY, offsetX, offsetY);

        // 读取物理属性（有覆盖用覆盖，无则用模板）
        double colScaleX = item.value("colScaleX", temp.colScaleX);
        double colScaleY = item.value("colScaleY", temp.colScaleY);
        entities[i].setCollisionScale(colScaleX, colScaleY);

        double colOffsetX = item.value("colOffsetX", temp.colOffsetX);
        double colOffsetY = item.value("colOffsetY", temp.colOffsetY);
        entities[i].setCollisionBoxOffset(colOffsetX, colOffsetY);

        int animSpeed = item.value("animSpeed", temp.animSpeed);
        if (animSpeed != -1)
        {
            entities[i].setAnimationSpeed(animSpeed);
        }

        // 登记到活跃名单中
        activeIndices.push_back(i);
    }


    // 3. 把剩下的空闲槽位全部归入“死亡空闲队列”，等以后动态生成时复用它们
    for (size_t i = activeCount; i < 200; i++)
    {
        entities[i].setIsAlive(false); // 标记死亡，占着坑但没启用
        deadIndices.push_back(i);
    }

    // 4. 为这一批初始活跃的演员们同步第一帧贴图与物理大小
    for (size_t idx : activeIndices)
    {
        entities[idx].initAnimationFromAnimator();
    }

    // 5. 构建哈希检索表，方便后续以 ID 查询位置
    rebuildMap();
    return true;
}

// 极其高效的导航查询：基于哈希表 O(1) 瞬间获取实体的内存地址
Entity* EntityManager::getEntity(EntityID id)
{
    auto it = idToIndex.find(id);
    if (it != idToIndex.end())
    {
        return &entities[it->second];
    }
    return nullptr;
}

// 极其高效的导航查询（常量版本）
const Entity* EntityManager::getEntity(EntityID id) const
{
    auto it = idToIndex.find(id);
    if (it != idToIndex.end())
    {
        return &entities[it->second];
    }
    return nullptr;
}

// 拿取实体数组大箱子的引用
std::vector<Entity>& EntityManager::getEntities()
{
    return entities;
}

// 拿取实体数组大箱子的引用（只读版）
const std::vector<Entity>& EntityManager::getEntities() const
{
    return entities;
}

// 清空大管家的所有资源
void EntityManager::clear()
{
    entities.clear();
    idToIndex.clear();
    spawnQueue.clear();
    activeIndices.clear();
    deadIndices.clear();
    nextEntityId = 1;
}

// 重新建立“ID -> vector下标”的对照表。
void EntityManager::rebuildMap()
{
    idToIndex.clear();
    for (size_t idx : activeIndices)
    {
        idToIndex[entities[idx].getId()] = idx;
    }
}

// 将实时生成的动态请求填入待处理小本子（存入请求队列，不当场生成以防指针失效）
void EntityManager::queueSpawnEntity(
    double x,
    double y,
    const std::string& templateName
)
{
    SpawnRequest req;
    req.x = x;
    req.y = y;
    req.templateName = templateName;

    spawnQueue.push_back(req);
}

// 核心帧末处理：垃圾回收已死实体，并处理动态新生实体
void EntityManager::processSpawns(AnimationClipManager& animationClips)
{
    // 步骤一：垃圾回收已经死掉的实体（使用 Swap-and-Pop 算法优化）
    for (size_t i = 0; i < activeIndices.size(); )
    {
        size_t idx = activeIndices[i];
        if (!entities[idx].getIsAlive()) // 如果它已经死了（比如被吃掉的硬币）
        {
            // 从哈希导航图中注销该 ID。
            idToIndex.erase(entities[idx].getId());
            
            // 归还到死亡索引列表中，以便将来别的请求复用这个坑位
            deadIndices.push_back(idx);

            // 核心操作：Swap-and-Pop！
            activeIndices[i] = activeIndices.back();
            activeIndices.pop_back();

            // 特别注意：此时不需要执行 ++i！
        }
        else
        {
            // 活得好好的，继续前进检查下一个
            ++i;
        }
    }

    // 步骤二：处理处于生成队列中的新实体请求（从 deadIndices 墓地名单里复用闲置槽位）
    if (!spawnQueue.empty())
    {
        for (const auto& req : spawnQueue)
        {
            if (deadIndices.empty())
            {
                std::cout << "警告：对象池爆满！无法在游戏中生成新演员！" << std::endl;
                break;
            }

            auto it = templates.find(req.templateName);
            if (it == templates.end())
            {
                std::cout << "警告：未找到模板 \"" << req.templateName << "\"，无法动态生成新演员！" << std::endl;
                continue;
            }
            const EntityTemplate& temp = it->second;

            // 从空闲名单里弹出一个可用的墓地槽位下标
            size_t idx = deadIndices.back();
            deadIndices.pop_back();

            EntityID iid = nextEntityId++;
            std::string generatedName = req.templateName;

            // 调用 reset 重塑这块槽位的属性
            entities[idx].reset(
                iid,
                generatedName,
                req.x,
                req.y,
                temp.controlled,
                temp.collidable,
                temp.blocking,
                temp.god,
                temp.type,
                req.templateName,
                true // 重新复活状态
            );

            // 初始化新实体的动画状态、朝向与片段映射
            entities[idx].initAnimations(
                temp.name,
                temp.initialAnim,
                temp.initialFacing,
                temp.stateToClip,
                animationClips
            );

            // 应用它要求的渲染大小和物理大小参数
            entities[idx].setSpriteTransform(temp.scaleX, temp.scaleY, temp.offsetX, temp.offsetY);
            entities[idx].setCollisionScale(temp.colScaleX, temp.colScaleY);
            entities[idx].setCollisionBoxOffset(temp.colOffsetX, temp.colOffsetY);

            if (temp.animSpeed != -1)
            {
                entities[idx].setAnimationSpeed(temp.animSpeed);
            }

            // 同步加载第一帧并同步精灵尺寸
            entities[idx].initAnimationFromAnimator();

            // 将其放回活人名单，并在哈希地图上重新注册它的名字。
            activeIndices.push_back(idx);
            idToIndex[iid] = idx;

            std::cout << "实时动态生成（复用对象池槽位 " << idx << "）：实体 \"" 
                      << generatedName << "\" (ID: " << iid << ") 已在世界坐标 (" << req.x << ", " << req.y << ") 顺利复活！" << std::endl;
        }
        
        // 记得清空本次请求本子，留待下一帧记录
        spawnQueue.clear();
    }
}

void EntityManager::collectSprites(RenderQueue& queue)
{
    for (size_t idx : activeIndices)
    {
        if (!entities[idx].getIsAlive())
        {
            continue;
        }

        queue.submit(entities[idx].getSprite(), SPRITE_TYPE_ENTITY, RGB(0, 220, 255));
    }
}
