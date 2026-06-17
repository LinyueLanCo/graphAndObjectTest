#include "EntityManager.h"
#include <fstream>
#include <iostream>
// 引入 nlohmann/json 第三方模板库。它通过非常高级的 C++ 模板元编程技术，
// 让我们能够像操作 Python 字典/JS 对象一样直接解析、读取和写入 JSON 配置文件。
// 我们在初始化关卡实体时，需要用它来解析 assets/data/entities.json 文件。
#include "json.hpp"

// 默认构造，暂时不需要做什么
EntityManager::EntityManager()
{
}

// 获取活跃实体索引名单，外部系统（如 Level 和 Renderer）都是通过它来做无空转遍历的
const std::vector<size_t>& EntityManager::getActiveIndices() const
{
    return activeIndices;
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

    // 声明一个 JSON 数据对象。nlohmann::json 是一个基于模板实现的类，
    // 内部使用复杂的数据结构（如 map/vector 嵌套）来表示任意 nested 的 JSON 节点。
    // 我们可以直接通过输入流操作符 `>>` 把文件流解析为树形结构。
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
    
    // 调用 std::unordered_map::clear() 清空导航地图。
    // 这项操作会释放哈希表内的所有存储节点（Buckets），保证重新载入关卡时，定位表完全干净，没有任何老实体的残留。
    nameToIndex.clear();
    spawnQueue.clear();

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
        std::string name = item.value("id", "");
        double x = item.value("x", 0.0);
        double y = item.value("y", 0.0);
        bool controlled = item.value("controlled", false);
        bool collidable = item.value("collidable", true);
        bool blocking = item.value("blocking", false);
        bool god = item.value("god", false);
        EntityType type = (EntityType)item.value("type", 4);
        AnimationSetId animSet = (AnimationSetId)item.value("animSet", 0);

        // 用 reset 函数重写这块槽位上实体的所有属性，“赋予新生命”
        entities[i].reset(
            name,
            x, y,
            controlled, collidable, blocking, god,
            type, animSet,
            1 // 初始活泼状态为 1（alive = 1）
        );

        // 读取 JSON 配置中的可选渲染参数，微调精灵绘制缩放与偏移
        double scaleX = item.value("scaleX", 1.0);
        double scaleY = item.value("scaleY", 1.0);
        double offsetX = item.value("offsetX", 0.0);
        double offsetY = item.value("offsetY", 0.0);
        entities[i].setSpriteTransform(scaleX, scaleY, offsetX, offsetY);

        // 读取可选的物理属性，微调碰撞盒大小和偏移
        double colScaleX = item.value("colScaleX", 1.0);
        double colScaleY = item.value("colScaleY", 1.0);
        entities[i].setCollisionScale(colScaleX, colScaleY);

        double colOffsetX = item.value("colOffsetX", 0.0);
        double colOffsetY = item.value("colOffsetY", 0.0);
        entities[i].setCollisionBoxOffset(colOffsetX, colOffsetY);

        int animSpeed = item.value("animSpeed", -1);
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

    // 4. 为这一批初始活跃的演员们同步载入动画素材贴图
    for (size_t idx : activeIndices)
    {
        entities[idx].initAnimationFromAnimator(animationClips);
    }

    // 5. 构建哈希检索表，方便后续以 ID 查询位置
    rebuildMap();
    return true;
}

// 极其高效的导航查询：基于哈希表 O(1) 瞬间获取实体的内存地址
Entity* EntityManager::getEntityById(const std::string& id)
{
    // 调用 std::unordered_map::find() 进行查找。
    // 该查找行为会计算 ID 字符串的哈希值，并在哈希表中瞬间定位其桶（Bucket）。
    // 在绝大多数情况下，时间复杂度是 O(1) 的常数级，极大提升了多实体环境下的寻址效率。
    auto it = nameToIndex.find(id);
    if (it != nameToIndex.end())
    {
        return &entities[it->second]; // 直接返回数组内实体的地址
    }
    return nullptr;
}

// 极其高效的导航查询（常量版本）
const Entity* EntityManager::getEntityById(const std::string& id) const
{
    // 同上，只读常量版本的哈希检索
    auto it = nameToIndex.find(id);
    if (it != nameToIndex.end())
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
    nameToIndex.clear();
    spawnQueue.clear();
    activeIndices.clear();
    deadIndices.clear();
}

// 重新建立“名字ID -> vector下标”的对照表。
// 当实体的活跃状态或索引发生大规模调整（例如关卡初次加载或大重构）后，
// 需要通过遍历 activeIndices 将所有活跃实体的 ID 映射到新的槽位，重新填入 nameToIndex 映射表中。
void EntityManager::rebuildMap()
{
    // 释放旧映射节点以准备填充新的一对一关联数据
    nameToIndex.clear();
    for (size_t idx : activeIndices)
    {
        // 同样是 O(1) 的插入效率
        nameToIndex[entities[idx].getId()] = idx;
    }
}

// 将实时生成的动态请求填入待处理小本子（存入请求队列，不当场生成以防指针失效）
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

// 核心帧末处理：垃圾回收已死实体，并处理动态新生实体
void EntityManager::processSpawns(AnimationClipManager& animationClips)
{
    // 步骤一：垃圾回收已经死掉的实体（使用 Swap-and-Pop 算法优化）
    // 如果直接从活跃数组中间删元素，后面所有元素往前挪的开销是 O(N)。
    // 我们的做法是把要删的元素和数组最后一个元素对调，然后把末尾弹掉，这样以 O(1) 的常数时间就可以搞定。
    for (size_t i = 0; i < activeIndices.size(); )
    {
        size_t idx = activeIndices[i];
        if (!entities[idx].getIsAlive()) // 如果它已经死了（比如被吃掉的硬币）
        {
            // 从哈希导航图中注销该 ID。
            // 使用 std::unordered_map::erase(key) 可以在 O(1) 的平均时间复杂度内定位并移出该键值对，
            // 保证已被回收的死亡实体不会再被任何外部查找访问到。
            nameToIndex.erase(entities[idx].getId());
            
            // 归还到死亡索引列表中，以便将来别的请求复用这个坑位
            deadIndices.push_back(idx);

            // 核心公式/操作：Swap-and-Pop！
            // 把尾巴上的元素调过来盖在当前被删的元素上
            activeIndices[i] = activeIndices.back();
            // 弹出尾部，完成物理删除
            activeIndices.pop_back();

            // 特别注意：此时不需要执行 ++i！
            // 因为被调过来的新元素现在躺在位置 i，我们必须在下一轮循环中再次检查位置 i，
            // 确保刚调过来的尾巴元素也是经过检测的。
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
                std::cout << "警告：对象池爆满！无法在游戏中生成新演员 \"" << req.id << "\"！" << std::endl;
                break;
            }

            // 从空闲名单里弹出一个可用的墓地槽位下标
            size_t idx = deadIndices.back();
            deadIndices.pop_back();

            // 调用 reset 重塑这块槽位的属性（借尸还魂）
            entities[idx].reset(
                req.id,
                req.x,
                req.y,
                req.controlled,
                req.collidable,
                req.blocking,
                req.god,
                req.type,
                req.animSet,
                1 // 重新复活状态
            );

            // 应用它要求的渲染大小和碰撞大小
            entities[idx].setSpriteTransform(req.scaleX, req.scaleY, 0.0, 0.0);
            entities[idx].setCollisionScale(req.colScaleX, req.colScaleY);

            if (req.animSpeed != -1)
            {
                entities[idx].setAnimationSpeed(req.animSpeed);
            }

            // 同步加载动画帧皮肤贴图
            entities[idx].initAnimationFromAnimator(animationClips);

            // 将其放回活人名单，并在哈希地图上重新注册它的名字。
            // 使用 std::unordered_map::operator[] 可以在 O(1) 的平均时间复杂度下插入新的映射对，
            // 它是典型的 C++ 关联式容器模板类操作，使得后续通过 ID 获取刚生成的实体同样飞快。
            activeIndices.push_back(idx);
            nameToIndex[req.id] = idx;

            std::cout << "实时动态生成（复用对象池槽位 " << idx << "）：实体 \"" 
                      << req.id << "\" 已在世界坐标 (" << req.x << ", " << req.y << ") 顺利复活！" << std::endl;
        }
        
        // 记得清空本次请求本子，留待下一帧记录
        spawnQueue.clear();
    }
}
