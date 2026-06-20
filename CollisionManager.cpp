#include "CollisionManager.h"
#include <cmath>
#include <iostream>
#include <algorithm>

// 检查两个二维矩形（a 和 b）有没有叠在一起
bool CollisionManager::isRectOverlapping(RectBox a, RectBox b)
{
    return !(a.left >= b.right ||  // a在b右侧
             a.right <= b.left ||  // a在b左侧
             a.bottom >= b.top ||  // a在b上方
             a.top <= b.bottom);   // a在b下方
}

// 检查两个一维线段（[aMin, aMax] 和 [bMin, bMax]）有没有重叠
bool CollisionManager::isRangeOverlapping(double aMin, double aMax, double bMin, double bMax)
{
    return !(aMax <= bMin || bMax <= aMin);
}

// 计算实体在水平 X 轴上，能走多远而不会穿墙
double CollisionManager::getAllowedMoveX(
    Entity& self,
    double moveX,
    std::vector<Entity>& entitys,
    const std::vector<size_t>& activeIndices,
    int selfIndex,
    TileMap& tileMap
)
{
    // 如果本来就没打算动，直接回 0
    if (moveX == 0)
    {
        return 0;
    }

    RectBox myBox = self.getWorldCollisionBox(); // 当前实体的碰撞盒子
    double allowedMove = moveX;                  // 假设没人挡，默认能走期望的距离

    // 先算算和其它“活跃且有阻挡”的实体之间的水平碰撞
    for (size_t idx : activeIndices)
    {
        if (idx == (size_t)selfIndex)
        {
            continue; // 排除自己和自己撞的乌龙
        }

        // 死亡实体或者没有阻挡属性的实体直接忽略
        if (!entitys[idx].getIsAlive() || !entitys[idx].isBlocking())
        {
            continue;
        }

        RectBox otherBox = entitys[idx].getWorldCollisionBox();

        // 核心判定：水平相撞的前提是，两个人在垂直 Y 轴上的范围必须是重叠的！
        if (!this->isRangeOverlapping(myBox.bottom, myBox.top, otherBox.bottom, otherBox.top))
        {
            continue;
        }

        // 1. 如果我们往右走（moveX > 0）
        if (moveX > 0)
        {
            // 只有当障碍物在我们的右侧，才可能会撞上
            if (otherBox.left >= myBox.right - EPS)
            {
                // 核心计算公式：两人的空隙距离 = 障碍物的左边界 - 我们的右边界
                double distance = otherBox.left - myBox.right;

                if (distance < 0)
                {
                    distance = 0; // 防止数值误差产生的微小重叠
                }

                // 允许移动的距离取“目前最窄的空隙”
                if (distance < allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
        // 2. 如果我们往左走（moveX < 0，注意此时位移和距离都是负数）
        else if (moveX < 0)
        {
            // 只有当障碍物在我们的左侧，才可能会撞上
            if (otherBox.right <= myBox.left + EPS)
            {
                // 核心计算公式：空隙距离 = 障碍物的右边界 - 我们的左边界
                double distance = otherBox.right - myBox.left;

                if (distance > 0)
                {
                    distance = 0;
                }

                // 负数越大（越接近0），说明限制越死，所以我们要用 max
                if (distance > allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
    }

    // 再算算和瓦片地图上的活跃瓦片之间的水平碰撞
    for (const TileInstance& tile : tileMap.getTileInstances())
    {
        if (!tile.visible || tile.tileId == TILE_EMPTY)
        {
            continue;
        }

        TileCollisionType collisionType = tile.collisionType;

        // 如果是空气或者单向平台，水平方向是不阻挡的，直接跳过
        if (collisionType == TILE_COLLISION_NONE ||
            collisionType == TILE_COLLISION_FULL_ONE_WAY ||
            collisionType == TILE_COLLISION_TOP_HALF_ONE_WAY ||
            collisionType == TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY ||
            collisionType == TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY)
        {
            continue;
        }

        RectBox tileBox = tileMap.getTileInstanceCollisionWorldBox(tile);

        // 同样，看垂直范围是否有交集
        if (!isRangeOverlapping(myBox.bottom, myBox.top, tileBox.bottom, tileBox.top))
        {
            continue;
        }

        // 计算方式和上面实体相撞完全一致
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

// 计算实体在垂直 Y 轴上，能走多远而不会卡进天花板或地下
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

    // 先算算和其它“活跃且有阻挡”的实体之间的垂直碰撞
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

        // 核心判定：垂直相撞的前提是，两个人在水平 X 轴上的范围必须是重叠的！
        if (!this->isRangeOverlapping(myBox.left, myBox.right, otherBox.left, otherBox.right))
        {
            continue;
        }

        // 1. 如果我们向上移动（跳起，moveY > 0）
        if (moveY > 0)
        {
            // 只有当障碍物在头顶上时
            if (otherBox.bottom >= myBox.top - EPS)
            {
                // 核心公式：头顶间距 = 障碍物下边缘 - 我们头顶上边缘
                double distance = otherBox.bottom - myBox.top;
                if (distance < 0) distance = 0;
                if (distance < allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
        // 2. 如果我们向下落（下坠，moveY < 0）
        else if (moveY < 0)
        {
            // 只有当障碍物在脚底下时
            if (otherBox.top <= myBox.bottom + EPS)
            {
                // 核心公式：脚底间距 = 障碍物上边缘 - 我们的脚底边缘
                double distance = otherBox.top - myBox.bottom;
                if (distance > 0) distance = 0;
                if (distance > allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
    }

    // 再算算和瓦片地图格子（包括实心墙、单向平台等）之间的垂直碰撞
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

        // 检查水平 X 轴区间是否有投影交集
        if (!isRangeOverlapping(myBox.left, myBox.right, tileBox.left, tileBox.right))
        {
            continue;
        }

        // 1. 如果我们向上跳（moveY > 0）
        if (moveY > 0)
        {
            // 特别注意：单向平台在往上跳时是不阻挡的！
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
        // 2. 如果我们向下落（moveY < 0）
        else if (moveY < 0)
        {
            // 核心：单向平台的单向判定逻辑！
            if (collisionType == TILE_COLLISION_FULL_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY)
            {
                // 获取未更新前的脚底高度值
                double prevFootY = myBox.bottom - moveY; 
                
                // 如果上一帧的脚底已经在平台高度之下了，说明他原本就在穿透中，这帧不应该挡他
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

// 步骤功能：强行锁边，防止演员掉出关卡下方或走到地图最左侧外面
void CollisionManager::limitInWorld(Entity& self, int worldWidth, int worldHeight)
{
    RectBox box = self.getWorldCollisionBox();

    // 1. 如果左出界了，强行推回来
    if (box.left < 0)
    {
        self.x += 0 - box.left;
        self.blockedByWorld = true;
    }

    box = self.getWorldCollisionBox();

    // 2. 如果右出界了，推回来
    if (box.right > worldWidth)
    {
        self.x -= box.right - worldWidth;
        self.blockedByWorld = true;
    }

    box = self.getWorldCollisionBox();

    // 3. 核心：如果落到底部深渊出界了，强行留在地上，并清空下坠速度
    if (box.bottom < 0)
    {
        self.y += 0 - box.bottom;
        self.blockedByWorld = true;
        self.onGround = true; // 强行标记踩在地面上

        if (self.velocityY < 0)
        {
            self.velocityY = 0; // 下坠速度归零
        }
    }

    box = self.getWorldCollisionBox();

    // 4. 如果撞到世界上方天花板了，推下来并清空上升冲劲
    if (box.top > worldHeight)
    {
        self.y -= box.top - worldHeight;
        self.blockedByWorld = true;

        if (self.velocityY > 0)
        {
            self.velocityY = 0; // 上升速度归零
        }
    }
}

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

void CollisionManager::updateOverlapEvents(EntityManager& entityManager)
{
    auto& entities = entityManager.getEntities();
    const auto& activeIndices = entityManager.getActiveIndices();
    std::vector<std::string> currentOverlapPairs;

    for (size_t i = 0; i < activeIndices.size(); i++)
    {
        for (size_t j = i + 1; j < activeIndices.size(); j++)
        {
            size_t idxA = activeIndices[i];
            size_t idxB = activeIndices[j];

            // 死亡槽位或者不参与碰撞的实体（比如上帝模式的角色）直接跳过
            if (!entities[idxA].getIsAlive() || !entities[idxB].getIsAlive())
            {
                continue;
            }

            if (!entities[idxA].isCollidable() || !entities[idxB].isCollidable())
            {
                continue;
            }

            // 获取两者本帧的世界碰撞盒边界
            RectBox a = entities[idxA].getWorldCollisionBox();
            RectBox b = entities[idxB].getWorldCollisionBox();

            // 计算两个矩形是否有重叠相交
            bool overlapping = isRectOverlapping(a, b);

            if (overlapping)
            {
                // 如果相撞，给两个实体标记上本帧 overlapping 标志（以供绘制红色碰撞盒）
                entities[idxA].setOverlapping(true);
                entities[idxB].setOverlapping(true);

                // 把对方的 ID 和类型登记到各自内部 of currentOverlaps vector 中
                entities[idxA].addOverlap(entities[idxB].getId(), entities[idxB].getEntityType());
                entities[idxB].addOverlap(entities[idxA].getId(), entities[idxA].getEntityType());

                // 将两者的 ID 按照大小排序拼接成一个唯一的键，避免重复日志
                std::string key = (entities[idxA].getId() < entities[idxB].getId()) ?
                                  (std::to_string(entities[idxA].getId()) + "_" + std::to_string(entities[idxB].getId())) :
                                  (std::to_string(entities[idxB].getId()) + "_" + std::to_string(entities[idxA].getId()));
                
                currentOverlapPairs.push_back(key);

                // 如果上一帧并没有碰过它，才打印日志（防止控制台疯狂刷屏）
                if (!contains(lastOverlapPairs, key))
                {
                    std::cout << "检测到新重叠事件：实体 [" << entities[idxA].getName() << "] (ID: " << entities[idxA].getId() 
                         << ") 碰到了 实体 [" << entities[idxB].getName() << "] (ID: " << entities[idxB].getId() << ")" << std::endl;
                }
            }
        }
    }
    // 保存这一帧的碰撞对记录
    lastOverlapPairs = currentOverlapPairs;
}

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

void CollisionManager::clearHistory()
{
    lastOverlapPairs.clear();
}
