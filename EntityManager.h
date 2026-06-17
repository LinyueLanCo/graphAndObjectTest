#pragma once

#include <vector>
#include <string>
#include <unordered_map> // 引入哈希映射容器（Hash Map）。因为我们需要通过实体的唯一名字 ID（string 类型）瞬间查到它在 entities 对象池数组中的下标（size_t 类型）。如果只用 vector 查找，就必须用 for 循环挨个比对名字，查找效率低（时间复杂度为 O(N)），而哈希映射表查找是瞬间完成的（时间复杂度为 O(1)）。
#include "Entity.h"
#include "AnimationClipManager.h"

// SpawnRequest: 动态生成实体的请求参数。
// 物理更新或者碰撞循环里如果直接加新实体，vector可能会扩容，导致实体指针或迭代器失效而闪退。
// 所以先把要生成的实体参数暂存到这个结构体里，等一帧结束了再统一安全地生成。
struct SpawnRequest
{
    std::string id;          // 给新实体起个唯一的名字（比如 "SpawnedCoin_1"）
    double x;                // 诞生位置的世界坐标 X
    double y;                // 诞生位置的世界坐标 Y
    bool controlled;         // 是否受玩家键盘控制
    bool collidable;         // 是否参与重叠判定（如吃金币）
    bool blocking;           // 是否作为实体阻挡物（如角色不能穿过另一个实体）
    bool god;                // 是否开启上帝模式（无视阻挡 and 重力）
    EntityType type;         // 实体类型（PLAYER, COIN, CHECKPOINT 等）
    AnimationSetId animSet;  // 绑定的动画皮肤资源 ID
    double scaleX;           // 渲染精灵的横向缩放比例
    double scaleY;           // 渲染精灵的纵向缩放比例
    double colScaleX;        // 物理碰撞盒的横向缩放比例
    double colScaleY;        // 物理碰撞盒的纵向缩放比例
    int animSpeed;           // 动画每帧切换的间隔 tick 数（-1 表示使用默认配置）
};

// EntityManager: 实体总管家，负责管理对象池和双索引。
// 它是这样管理实体的：
// 1. 初始化时，直接把大容器 entities 扩容到 200 个，之后游戏运行期间绝对不扩容或缩水，确保物理地址固定防闪退。
// 2. activeIndices 存的是当前活着实体的下标，外部遍历只看这里。
// 3. deadIndices 存的是空闲可用的槽位下标，新生成的实体直接去里面拿个位置 reset 即可。
// 4. nameToIndex 建立名字到下标的映射导航，方便通过 ID O(1) 快速定位实体。
class EntityManager
{
private:
    std::vector<Entity> entities;                        // 对象池本尊：固定存放 200 个实体实例的连续大箱子
    // 导航地图：记录实体 ID 到大箱子下标的映射。
    // 特别使用 std::unordered_map，是因为它基于哈希表实现，在查找时具有 O(1) 的平均时间复杂度。
    // 当我们需要频繁通过实体的 ID（如 "Player1"）来定位并操作具体的实体时，使用哈希表可以实现瞬间定位，
    // 避免了为了找一个实体而不得不遍历整个 entities 容器（在 vector 中查找是 O(N) 复杂度），极大地节省了 CPU 时间开销。
    std::unordered_map<std::string, size_t> nameToIndex; 
    std::vector<SpawnRequest> spawnQueue;                // 临时寄存处：本帧内请求动态生成但还没落地的演员队列

    // 双索引列表，高效遍历与复用的核心
    std::vector<size_t> activeIndices;                   // 活跃索引名单：目前活在游戏世界里的实体下标
    std::vector<size_t> deadIndices;                     // 空闲索引名单：死掉或者还没启用的槽位下标

public:
    EntityManager();

    // 简单获取活跃列表引用，只读不写，外部渲染和物理更新直接循环它即可
    const std::vector<size_t>& getActiveIndices() const;

    // 从 JSON 配置文件载入初始关卡的实体
    // filepath: JSON 配置文件路径（比如 "assets/data/entities.json"）
    // animationClips: 动画素材大管家（用来给加载进来的演员绑定动画贴图）
    bool loadEntities(const std::string& filepath, AnimationClipManager& animationClips);

    // 把实时生成新实体的请求放入队列，等帧末安全处理
    // 参数意义：
    //   id: 实体的唯一名字 ID
    //   x, y: 新生实体的世界坐标位置
    //   controlled: 是否由玩家控制
    //   collidable: 是否能发生重叠判定
    //   blocking: 是否有实体阻挡体积
    //   god: 是否开启上帝模式
    //   type: 实体类型标识
    //   animSet: 使用的动画皮肤包 ID
    //   scaleX, scaleY: 图片渲染的宽高缩放比例
    //   colScaleX, colScaleY: 物理碰撞盒的缩放比例
    //   animSpeed: 动画播放帧间隔 tick
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

    // 帧末安全大扫除：
    // 1. 回收 `isAlive == false` 的活跃槽位放入 deadIndices（Swap-and-Pop 极速回收）。
    // 2. 从 deadIndices 取空闲槽位重置并复活 spawnQueue 中的生成请求。
    // animationClips: 用来给新生实体配发对应的动画片段
    void processSpawns(AnimationClipManager& animationClips);

    // 两个极速 ID 导航函数，支持通过名字拿取实体的读写/只读指针（找不到就回 nullptr）
    Entity* getEntityById(const std::string& id);
    const Entity* getEntityById(const std::string& id) const;

    // 兼容老系统的 entities 数组引用接口，不推荐频繁遍历
    std::vector<Entity>& getEntities();
    const std::vector<Entity>& getEntities() const;

    // 清空大管家状态，清空所有队列和索引
    void clear();

    // 根据活跃名单，重新构建 nameToIndex 导航图
    void rebuildMap();
};
