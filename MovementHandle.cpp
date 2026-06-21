#include "MovementHandle.h"
#include "MathUtils.h"

// 根据行为意图、物理规则和碰撞检测结果，更新当前实体的移动位置和速度状态。
void MovementHandle::update(
    Entity& self,
    BehaviorIntent intent,
    std::vector<Entity>& entitys,
    const std::vector<size_t>& activeIndices,
    int selfIndex,
    TileMap& tileMap,
    int worldWidth,
    int worldHeight,
    CollisionManager& collisionManager
)
{
    // MovementHandle 本帧数据更新流程：
    // 1. 获取水平和垂直移动输入，判断是否开启跑步冲刺。
    // 2. 如果是上帝模式，直接归一化向量移动并限制在地图内，不受重力和碰撞阻挡影响。
    // 3. 普通模式下，先处理水平期望位移并作碰撞修正，更新 X 坐标。
    // 4. 接着累加重力速度，处理跳跃意图，计算期望的 Y 位移并做阻挡判定，更新 Y 坐标。

    double inputX = intent.moveX;
    double inputY = intent.moveY;

    double currentSpeed = self.speed; // 拿到实体默认的移动速度（基础值）

    bool hasMoveInput = (inputX != 0);

    // 1. 更新朝向：只有在玩家有按键输入时才改变朝向，不按键时保持上一帧的朝向
    if (inputX < -EPS)
    {
        self.setFacingDirection(LEFT); // 往左看
    }
    else if (inputX > EPS)
    {
        self.setFacingDirection(RIGHT); // 往右看
    }

    // 2. 判定是否处于冲刺狂奔状态
    bool wantSprint = (intent.wantSprint && hasMoveInput);

    if (!wantSprint)
    {
        self.sprinting = false; // 没按跑步键，或者没走动，取消冲刺
    }
    else
    {
        // 只有当人在地面上时，才可以开启狂奔冲刺
        if (!self.sprinting && self.onGround)
        {
            self.sprinting = true;
        }
    }

    // 冲刺状态下，速度翻倍！
    if (self.sprinting)
    {
        currentSpeed = self.speed * 2;
    }

    // 上帝模式的特殊飞行逻辑：不管碰撞和重力，直接飞过去。
    if (self.god)
    {
        MathUtils::normalize2D(inputX, inputY);

        self.x += inputX * currentSpeed;
        self.y += inputY * currentSpeed;

        // 依然要限制不能飞到屏幕外面去，调用 collisionManager 进行强制锁边
        collisionManager.limitInWorld(self, worldWidth, worldHeight);
        return;
    }

    // 普通物理移动逻辑（受重力、阻挡碰撞和跳跃控制）

    // 1. 处理起跳意图：只有在踩着地面（onGround）时，才允许弹起
    if (intent.wantJump && self.onGround)
    {
        self.velocityY = self.jumpSpeed; // 给一个向上的跳跃初速度
        self.onGround = false;       // 瞬间腾空
        self.InAir = true;           // 悬空标记
        self.jumping = true;         // 跳跃状态激活
    }

    // 2. 水平移动处理（X轴）
    // wantMoveX = 期望方向 * 速度（1帧按单位时间计算）
    double wantMoveX = inputX * currentSpeed;

    // 测算实际被地图和其它人阻挡后的允许移动距离
    double allowedMoveX = collisionManager.getAllowedMoveX(
        self,
        wantMoveX,
        entitys,
        activeIndices,
        selfIndex,
        tileMap
    );
    
    // 如果允许移动的距离比你想移动的距离小，说明有东西挡住你了
    if (fabs(allowedMoveX - wantMoveX) > EPS)
    {
        self.blockedByEntity = true; // 触发被阻挡标记
        self.collisionState = true;  // 触发碰撞反馈（碰撞盒变红）
    }

    self.x += allowedMoveX; // 真正把位移加到世界坐标上

    // 3. 垂直移动处理（Y轴）
    // 速度公式：本帧速度 = 上帧速度 - 重力（velocityY 随时间越来越小，形成坠落）
    self.velocityY -= self.gravity;

    // 物理限制：落体速度不能超过终端最大速度（MAX_FALL_SPEED 为负数，比如 -16 像素/帧）
    if (self.velocityY < MAX_FALL_SPEED)
    {
        self.velocityY = MAX_FALL_SPEED;
    }

    self.onGround = false; // 默认这帧先不算在地上，由接下来的阻挡修正来判断是否落地
    self.InAir = true;

    double wantMoveY = self.velocityY;

    // 测算这帧落下去或者升上去时，会不会撞到格子天花板或者地板
    double allowedMoveY = collisionManager.getAllowedMoveY(
        self,
        wantMoveY,
        entitys,
        activeIndices,
        selfIndex,
        tileMap
    );
    
    // 如果实际位移跟期望位移不一致，说明垂直方向“触底”或者“撞顶”了
    if (fabs(allowedMoveY - wantMoveY) > EPS)
    {
        if (wantMoveY < 0)
        {
            // 如果是在往下落的时候被截断，说明“稳稳落地”了！
            self.onGround = true;
            self.InAir = false;
            self.jumping = false; // 落地后跳跃动作宣告结束
        }
        else if (wantMoveY > 0)
        {
            // 如果是往上跳时被截断，说明“撞天花板了”！
            self.blockedByEntity = true;
            self.collisionState = true;
        }

        self.velocityY = 0; // 落地或者撞顶时，把垂直速度归零
    }

    self.y += allowedMoveY; // 真正把高度变化应用到坐标上

    // 4. 最后做一次锁屏限制，不掉出关卡边界
    collisionManager.limitInWorld(self, worldWidth, worldHeight);
}
