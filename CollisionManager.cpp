#include "CollisionManager.h"
#include <cmath>
#include <iostream>
#include <algorithm>

// 检查两个二维矩形（a 和 b）的 AABB 包围盒在水平和垂直投影上是否重叠。
// 如果一个矩形的最左侧大于等于另一个的最右侧，或者最右侧小于等于另一个的最左侧，则两者在水平投影上不重叠。
// 同理，垂直投影也进行类似计算，若有一个轴向不重叠，则整体不重叠。
bool CollisionManager::isRectOverlapping(RectBox a, RectBox b)
{
    return !(a.left >= b.right ||  // a在b右侧，无水平重叠
             a.right <= b.left ||  // a在b左侧，无水平重叠
             a.bottom >= b.top ||  // a在b上方，无垂直重叠
             a.top <= b.bottom);   // a在b下方，无垂直重叠
}

// 检查两个一维闭区间（[aMin, aMax] 和 [bMin, bMax]）有没有重叠。
// 如果第一个区间的最大值小于等于第二个区间的最小值，或者第二个区间的最大值小于等于第一个区间的最小值，则无交集。
bool CollisionManager::isRangeOverlapping(double aMin, double aMax, double bMin, double bMax)
{
    return !(aMax <= bMin || bMax <= aMin);
}

// 计算实体在水平 X 轴上，本帧能移动的最大安全距离（像素值），用于防止水平穿过实心瓦片或有阻挡属性的其它实体。
// std::vector 是标准动态数组，在这里主要用于传递活跃实体的集合以及索引。
double CollisionManager::getAllowedMoveX(
    Entity& self,
    double moveX,
    std::vector<Entity>& entitys,
    const std::vector<size_t>& activeIndices,
    int selfIndex,
    TileMap& tileMap
)
{
    // 如果没有移动意图，直接返回 0 距离
    if (moveX == 0)
    {
        return 0;
    }

    RectBox myBox = self.getWorldCollisionBox(); // 获取当前实体在世界坐标系下的包围盒
    double allowedMove = moveX;                  // 默认允许移动完整期望距离

    // 遍历所有活着的有阻挡属性的实体，计算潜在的碰撞并限制移动距离
    for (size_t idx : activeIndices)
    {
        if (idx == (size_t)selfIndex)
        {
            continue; // 跳过当前实体自身，避免自己和自己碰撞
        }

        // 跳过已死实体和不具备阻挡属性的非实体对象
        if (!entitys[idx].getIsAlive() || !entitys[idx].isBlocking())
        {
            continue;
        }

        RectBox otherBox = entitys[idx].getWorldCollisionBox();

        // 只有当两个实体在 Y 轴投影上发生了重叠相交时，水平移动才可能和它相撞
        if (!this->isRangeOverlapping(myBox.bottom, myBox.top, otherBox.bottom, otherBox.top))
        {
            continue;
        }

        // 往右侧移动时（moveX 值为正）
        if (moveX > 0)
        {
            // 只有当障碍物实体的左边界在我们的右边界以右时（加上小量 EPS 防止浮点数精度误差卡死）才计算阻挡
            if (otherBox.left >= myBox.right - EPS)
            {
                // 计算两个实体边界的实际物理间距
                double distance = otherBox.left - myBox.right;

                if (distance < 0)
                {
                    distance = 0; // 消除浮点数微小误差导致的负间距
                }

                // 更新安全移动像素为当前求得的最窄间距
                if (distance < allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
        // 往左侧移动时（moveX 值为负）
        else if (moveX < 0)
        {
            // 只有当障碍物实体的右边界在我们的左边界以左时，才可能迎面撞上
            if (otherBox.right <= myBox.left + EPS)
            {
                // 计算两个实体边界的物理间距，左移时为负值
                double distance = otherBox.right - myBox.left;

                if (distance > 0)
                {
                    distance = 0;
                }

                // 负数越大越接近 0，即阻挡越严，因此采用 std::max 进行限制
                if (distance > allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
    }

    // 遍历地图上的所有格子瓦片，计算地形障碍的水平碰撞阻挡
    for (const TileInstance& tile : tileMap.getTileInstances())
    {
        // 忽略不可见的空瓦片
        if (!tile.visible || tile.tileId == TILE_EMPTY)
        {
            continue;
        }

        TileCollisionType collisionType = tile.collisionType;

        // 空气图块和单向平台没有水平阻挡，直接跳过
        if (collisionType == TILE_COLLISION_NONE ||
            collisionType == TILE_COLLISION_FULL_ONE_WAY ||
            collisionType == TILE_COLLISION_TOP_HALF_ONE_WAY ||
            collisionType == TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY ||
            collisionType == TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY)
        {
            continue;
        }

        RectBox tileBox = tileMap.getTileInstanceCollisionWorldBox(tile);

        // 如果垂直 Y 轴投影区间没有发生重叠，则不可能在水平方向撞上该瓦片
        if (!isRangeOverlapping(myBox.bottom, myBox.top, tileBox.bottom, tileBox.top))
        {
            continue;
        }

        // 与前面实体的碰撞计算逻辑相同，根据移动方向钳制移动偏移值
        if (moveX > 0)
        {
            if (tileBox.left >= myBox.right - EPS)
            {
                double distance = tileBox.left - myBox.right;
                if (distance < 0) distance = 0;
                if (distance < allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
        else if (moveX < 0)
        {
            if (tileBox.right <= myBox.left + EPS)
            {
                double distance = tileBox.right - myBox.left;
                if (distance > 0) distance = 0;
                if (distance > allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
    }

    return allowedMove;
}

// 计算实体在垂直 Y 轴上，本帧能移动的最大安全距离（像素值），用于防穿墙或站立判定。
// std::vector 是标准动态数组，在这里主要用于传递活跃实体的集合以及索引。
double CollisionManager::getAllowedMoveY(
    Entity& self,
    double moveY,
    std::vector<Entity>& entitys,
    const std::vector<size_t>& activeIndices,
    int selfIndex,
    TileMap& tileMap
)
{
    if (moveY == 0)
    {
        return 0;
    }

    RectBox myBox = self.getWorldCollisionBox();
    double allowedMove = moveY;

    // 遍历计算与其它活跃、有阻挡属性实体的垂直方向碰撞
    for (size_t idx : activeIndices)
    {
        if (idx == (size_t)selfIndex)
        {
            continue;
        }

        if (!entitys[idx].getIsAlive() || !entitys[idx].isBlocking())
        {
            continue;
        }

        RectBox otherBox = entitys[idx].getWorldCollisionBox();

        // 只有两个实体在 X 轴的投影区间重叠时，垂直移动才会相撞
        if (!this->isRangeOverlapping(myBox.left, myBox.right, otherBox.left, otherBox.right))
        {
            continue;
        }

        // 向上移动（跳跃，moveY 为正值）
        if (moveY > 0)
        {
            // 头顶上的障碍物包围盒下边缘应在实体头顶上边缘以北，计入浮点数公差
            if (otherBox.bottom >= myBox.top - EPS)
            {
                double distance = otherBox.bottom - myBox.top;
                if (distance < 0) distance = 0;
                if (distance < allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
        // 向下移动（降落，moveY 为负值）
        else if (moveY < 0)
        {
            // 脚底下的障碍物包围盒上边缘应在实体脚底边缘以南，计入浮点数公差
            if (otherBox.top <= myBox.bottom + EPS)
            {
                double distance = otherBox.top - myBox.bottom;
                if (distance > 0) distance = 0;
                if (distance > allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
    }

    // 遍历计算与地图瓦片网格之间的垂直方向阻挡，并专门处理单向平台的单向拦截逻辑
    for (const TileInstance& tile : tileMap.getTileInstances())
    {
        if (!tile.visible || tile.tileId == TILE_EMPTY)
        {
            continue;
        }

        TileCollisionType collisionType = tile.collisionType;

        if (collisionType == TILE_COLLISION_NONE)
        {
            continue;
        }

        RectBox tileBox = tileMap.getTileInstanceCollisionWorldBox(tile);

        // 如果两个实体在水平 X 轴上没有投影重叠，直接忽略此瓦片
        if (!isRangeOverlapping(myBox.left, myBox.right, tileBox.left, tileBox.right))
        {
            continue;
        }

        // 向上跳跃时（moveY 为正）
        if (moveY > 0)
        {
            // 单向平台瓦片从下方穿透时，完全不产生头顶碰撞拦截，直接跳过
            if (collisionType == TILE_COLLISION_FULL_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY)
            {
                continue;
            }

            if (tileBox.bottom >= myBox.top - EPS)
            {
                double distance = tileBox.bottom - myBox.top;
                if (distance < 0) distance = 0;
                if (distance < allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
        // 向下坠落时（moveY 为负）
        else if (moveY < 0)
        {
            // 针对单向平台的拦截过滤逻辑：
            // 如果实体的上一帧未更新前的脚底高度已经位于单向平台上边缘之下，说明是从平台侧向或者穿透落入其中的，此时不应当对其进行拦截
            if (collisionType == TILE_COLLISION_FULL_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY)
            {
                double prevFootY = myBox.bottom - moveY; 
                
                if (prevFootY < tileBox.top - EPS)
                {
                    continue;
                }
            }

            if (tileBox.top <= myBox.bottom + EPS)
            {
                double distance = tileBox.top - myBox.bottom;
                if (distance > 0) distance = 0;
                if (distance > allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
    }

    return allowedMove;
}

// 强制限位：当实体越过关卡世界的物理总尺寸边缘时，强行推回包围盒内，并修正其对应的物理状态
void CollisionManager::limitInWorld(Entity& self, int worldWidth, int worldHeight)
{
    RectBox box = self.getWorldCollisionBox();

    // 限制左侧边界：如果包围盒左边界小于 0，向右偏移修正
    if (box.left < 0)
    {
        self.x += 0 - box.left;
        self.blockedByWorld = true;
    }

    box = self.getWorldCollisionBox();

    // 限制右侧边界：如果包围盒右边界超过世界总像素宽度，向左偏移修正
    if (box.right > worldWidth)
    {
        self.x -= box.right - worldWidth;
        self.blockedByWorld = true;
    }

    box = self.getWorldCollisionBox();

    // 限制底部边界：如果包围盒底部低于 0（落入深渊），向顶修正，并强制重置速度和着地标志 onGround
    if (box.bottom < 0)
    {
        self.y += 0 - box.bottom;
        self.blockedByWorld = true;
        self.onGround = true; 

        if (self.velocityY < 0)
        {
            self.velocityY = 0; 
        }
    }

    box = self.getWorldCollisionBox();

    // 限制顶部边界：如果包围盒顶部超出世界最大高度，推回复位，并消除向上的垂直速度
    if (box.top > worldHeight)
    {
        self.y -= box.top - worldHeight;
        self.blockedByWorld = true;

        if (self.velocityY > 0)
        {
            self.velocityY = 0; 
        }
    }
}

// 判断指定的 key 字符串是否存放在动态数组 vec 容器中
bool CollisionManager::contains(const std::vector<std::string>& vec, const std::string& key)
{
    for (const auto& item : vec)
    {
        if (item == key)
        {
            return true;
        }
    }
    return false;
}

// 双重循环对所有活着的实体进行 AABB 碰撞重叠分析，更新它们内部的重叠信息列表并进行碰撞日志去重输出
void CollisionManager::updateOverlapEvents(EntityManager& entityManager)
{
    auto& entities = entityManager.getEntities();
    const auto& activeIndices = entityManager.getActiveIndices();
    
    // std::vector 是用来保存本帧所有产生的碰撞对关键字的动态数组容器
    std::vector<std::string> currentOverlapPairs;

    // 两两不重复组合遍历，索引为活跃实体在对象池中的下标
    for (size_t i = 0; i < activeIndices.size(); i++)
    {
        for (size_t j = i + 1; j < activeIndices.size(); j++)
        {
            size_t idxA = activeIndices[i];
            size_t idxB = activeIndices[j];

            // 跳过已死亡的实体
            if (!entities[idxA].getIsAlive() || !entities[idxB].getIsAlive())
            {
                continue;
            }

            // 过滤无碰撞属性的实体
            if (!entities[idxA].isCollidable() || !entities[idxB].isCollidable())
            {
                continue;
            }

            RectBox a = entities[idxA].getWorldCollisionBox();
            RectBox b = entities[idxB].getWorldCollisionBox();

            // 进行 AABB 碰撞检测
            bool overlapping = isRectOverlapping(a, b);

            if (overlapping)
            {
                // 将重叠状态标记到对应的实体中，用于后续调试框绘制红色标识
                entities[idxA].setOverlapping(true);
                entities[idxB].setOverlapping(true);

                // 相互添加对方的实体信息到各自的当前帧重叠记录容器中
                entities[idxA].addOverlap(entities[idxB].getId(), entities[idxB].getEntityType());
                entities[idxB].addOverlap(entities[idxA].getId(), entities[idxA].getEntityType());

                // 生成拼接键，按照 ID 排序保证唯一性（如 "12_18"），以便跨帧去重
                // std::to_string 函数来自 <string> 头文件，用于将数值转换为 std::string 字符序列
                std::string key = (entities[idxA].getId() < entities[idxB].getId()) ?
                                  (std::to_string(entities[idxA].getId()) + "_" + std::to_string(entities[idxB].getId())) :
                                  (std::to_string(entities[idxB].getId()) + "_" + std::to_string(entities[idxA].getId()));
                
                currentOverlapPairs.push_back(key);

                // 如果上一帧并没有记录过此碰撞对，说明是本帧才发生的全新重叠，打印控制台输出
                if (!contains(lastOverlapPairs, key))
                {
                    std::cout << "检测到新重叠事件：实体 [" << entities[idxA].getName() << "] (ID: " << entities[idxA].getId() 
                         << ") 碰到了 实体 [" << entities[idxB].getName() << "] (ID: " << entities[idxB].getId() << ")" << std::endl;
                }
            }
        }
    }
    // 更新历史重叠配对缓存
    lastOverlapPairs = currentOverlapPairs;
}

// 调度所有活跃实体根据已检测到的重叠事件容器，各自调用其 resolveOverlaps 自治更新功能进行业务反馈
void CollisionManager::resolveEntityOverlaps(EntityManager& entityManager)
{
    auto& entities = entityManager.getEntities();
    const auto& activeIndices = entityManager.getActiveIndices();
    for (size_t idx : activeIndices)
    {
        if (entities[idx].getIsAlive())
        {
            entities[idx].resolveOverlaps(entityManager);
        }
    }
}

// 清除所有的历史重叠配对缓存，释放 std::vector 容器占用的内存并清空其元素数量为 0
void CollisionManager::clearHistory()
{
    lastOverlapPairs.clear();
}
