#include "CollisionHandle.h"

// 功能：判断两个 AABB 矩形是否真正重叠。
bool CollisionHandle::isRectOverlapping(RectBox a, RectBox b)
{
    if (a.right <= b.left + EPS)
    {
        return false;
    }

    if (a.left >= b.right - EPS)
    {
        return false;
    }

    if (a.top <= b.bottom + EPS)
    {
        return false;
    }

    if (a.bottom >= b.top - EPS)
    {
        return false;
    }

    return true;
}

// 功能：判断两个一维区间是否真正重叠。
bool CollisionHandle::isRangeOverlapping(
    double aMin,
    double aMax,
    double bMin,
    double bMax
)
{
    if (aMax <= bMin + EPS)
    {
        return false;
    }

    if (aMin >= bMax - EPS)
    {
        return false;
    }

    return true;
}

// 功能：计算实体在 X 轴上不会穿透阻挡物的最大允许位移。
double CollisionHandle::getAllowedMoveX(
    Entity& self,
    double moveX,
    vector<Entity>& entitys,
    int selfIndex,
    TileMap& tileMap
)
{
    /*
    X 轴阻挡修正逻辑：
        输入：moveX = 本帧期望水平位移
        输出：allowedMove = 本帧真正允许移动的水平距离

    核心思路：
        1. 先取得当前实体的 AABB：myBox
        2. 遍历所有 blocking 实体
        3. 如果 Y 轴范围不重叠，说明上下错开，不可能水平撞到，跳过
        4. 如果向右移动，只看位于右侧的障碍：
              distance = other.left - myBox.right
              allowedMove = min(allowedMove, distance)
        5. 如果向左移动，只看位于左侧的障碍：
              distance = other.right - myBox.left
              allowedMove = max(allowedMove, distance)  // moveX 为负数，所以取更接近 0 的限制值

    它和 overlap 有关，但不是简单地“先移动到下一帧再判断 overlap”。
    它是在当前帧提前计算到障碍边缘的距离，防止下一帧真正重叠。
    */
    if (moveX == 0)
    {
        return 0;
    }

    RectBox myBox = self.getWorldCollisionBox();
    double allowedMove = moveX;

    for (int i = 0; i < (int)entitys.size(); i++)
    {
        if (i == selfIndex)
        {
            continue;
        }

        // 死亡实体不再参与阻挡计算。
        // 否则被 killEntity() 的阻挡物仍可能继续挡住玩家。
        if (!entitys[i].getIsAlive())
        {
            continue;
        }

        if (!entitys[i].isBlocking())
        {
            continue;
        }

        RectBox otherBox = entitys[i].getWorldCollisionBox();

        if (!this->isRangeOverlapping(myBox.bottom, myBox.top, otherBox.bottom, otherBox.top))
        {
            continue;
        }

        if (moveX > 0)
        {
            if (otherBox.left >= myBox.right - EPS)
            {
                double distance = otherBox.left - myBox.right;

                if (distance < 0)
                {
                    distance = 0;
                }

                if (distance < allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
        else if (moveX < 0)
        {
            if (otherBox.right <= myBox.left + EPS)
            {
                double distance = otherBox.right - myBox.left;

                if (distance > 0)
                {
                    distance = 0;
                }

                if (distance > allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
    }

    for (int row = 0; row < tileMap.getRows(); row++)
    {
        for (int col = 0; col < tileMap.getCols(); col++)
        {
            TileCollisionType collisionType = tileMap.getTileCollisionType(row, col);

            if (collisionType == TILE_COLLISION_NONE)
            {
                continue;
            }

            if (collisionType == TILE_COLLISION_FULL_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY)
            {
                continue;
            }

            RectBox tileBox = tileMap.getTileCollisionWorldBox(row, col);

            if (!isRangeOverlapping(myBox.bottom, myBox.top, tileBox.bottom, tileBox.top))
            {
                continue;
            }

            if (moveX > 0)
            {
                if (tileBox.left >= myBox.right - EPS)
                {
                    double distance = tileBox.left - myBox.right;

                    if (distance < 0)
                    {
                        distance = 0;
                    }

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

                    if (distance > 0)
                    {
                        distance = 0;
                    }

                    if (distance > allowedMove)
                    {
                        allowedMove = distance;
                    }
                }
            }
        }
    }

    return allowedMove;
}

// 功能：计算实体在 Y 轴上不会穿透阻挡物的最大允许位移。
double CollisionHandle::getAllowedMoveY(
    Entity& self,
    double moveY,
    vector<Entity>& entitys,
    int selfIndex,
    TileMap& tileMap
)
{
    /*
    Y 轴阻挡修正逻辑：
        输入：moveY = 本帧期望垂直位移，通常来自 velocityY
        输出：allowedMove = 本帧真正允许移动的垂直距离

    核心思路：
        1. 先取得当前实体的 AABB：myBox
        2. 遍历所有 blocking 实体
        3. 如果 X 轴范围不重叠，说明左右错开，不可能垂直撞到，跳过
        4. 如果向上移动，只看位于上方的障碍：
              distance = other.bottom - myBox.top
              allowedMove = min(allowedMove, distance)
        5. 如果向下移动，只看位于下方的障碍：
              distance = other.top - myBox.bottom
              allowedMove = max(allowedMove, distance)  // moveY 为负数

    下落时 allowedMoveY 与 wantMoveY 不一致，通常意味着落地；
    上升时不一致，通常意味着撞到上方阻挡物。
    */
    if (moveY == 0)
    {
        return 0;
    }

    RectBox myBox = self.getWorldCollisionBox();
    double allowedMove = moveY;

    for (int i = 0; i < (int)entitys.size(); i++)
    {
        if (i == selfIndex)
        {
            continue;
        }

        // 死亡实体不再参与阻挡计算。
        // 否则被 killEntity() 的阻挡物仍可能继续挡住玩家。
        if (!entitys[i].getIsAlive())
        {
            continue;
        }

        if (!entitys[i].isBlocking())
        {
            continue;
        }

        RectBox otherBox = entitys[i].getWorldCollisionBox();

        if (!this->isRangeOverlapping(myBox.left, myBox.right, otherBox.left, otherBox.right))
        {
            continue;
        }

        if (moveY > 0)
        {
            if (otherBox.bottom >= myBox.top - EPS)
            {
                double distance = otherBox.bottom - myBox.top;

                if (distance < 0)
                {
                    distance = 0;
                }

                if (distance < allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
        else if (moveY < 0)
        {
            if (otherBox.top <= myBox.bottom + EPS)
            {
                double distance = otherBox.top - myBox.bottom;

                if (distance > 0)
                {
                    distance = 0;
                }

                if (distance > allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
    }

    for (int row = 0; row < tileMap.getRows(); row++)
    {
        for (int col = 0; col < tileMap.getCols(); col++)
        {
            TileCollisionType collisionType = tileMap.getTileCollisionType(row, col);

            if (collisionType == TILE_COLLISION_NONE)
            {
                continue;
            }

            if (collisionType == TILE_COLLISION_FULL_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY)
            {
                if (moveY >= 0)
                {
                    continue;
                }
            }

            RectBox tileBox = tileMap.getTileCollisionWorldBox(row, col);

            if (!isRangeOverlapping(myBox.left, myBox.right, tileBox.left, tileBox.right))
            {
                continue;
            }

            if (moveY > 0)
            {
                if (tileBox.bottom >= myBox.top - EPS)
                {
                    double distance = tileBox.bottom - myBox.top;

                    if (distance < 0)
                    {
                        distance = 0;
                    }

                    if (distance < allowedMove)
                    {
                        allowedMove = distance;
                    }
                }
            }
            else if (moveY < 0)
            {
                if (tileBox.top <= myBox.bottom + EPS)
                {
                    double distance = tileBox.top - myBox.bottom;

                    if (distance > 0)
                    {
                        distance = 0;
                    }

                    if (distance > allowedMove)
                    {
                        allowedMove = distance;
                    }
                }
            }
        }
    }

    return allowedMove;
}

// 功能：把实体限制在世界边界内并修正相关物理状态。
void CollisionHandle::limitInWorld(
    Entity& self,
    int worldWidth,
    int worldHeight
)
{
    /*
    世界边界修正：
        这个函数把实体限制在 [0, worldWidth] x [0, worldHeight] 内。
        如果实体超出边界，就把它推回边界内。

    重要状态反馈：
        - 撞到世界边界时 blockedByWorld = true
        - 撞到底部边界时，视为站在地面：
              onGround = true
              InAir = false
              jumping = false
              向下速度 velocityY 清零
    */
    RectBox box = self.getWorldCollisionBox();

    if (box.left < 0)
    {
        self.x += 0 - box.left;
        self.blockedByWorld = true;
    }

    box = self.getWorldCollisionBox();

    if (box.right > worldWidth)
    {
        self.x -= box.right - worldWidth;
        self.blockedByWorld = true;
    }

    box = self.getWorldCollisionBox();

    if (box.bottom < 0)
    {
        self.y += 0 - box.bottom;
        self.blockedByWorld = true;
        self.onGround = true;
        self.InAir = false;
        self.jumping = false;

        if (self.velocityY < 0)
        {
            self.velocityY = 0;
        }
    }

    box = self.getWorldCollisionBox();

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
