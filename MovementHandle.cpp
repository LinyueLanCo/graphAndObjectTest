#include "MovementHandle.h"

// 功能：根据行为意图、物理规则和碰撞结果更新实体移动状态。
void MovementHandle::update(
    Entity& self,
    BehaviorIntent intent,
    vector<Entity>& entitys,
    int selfIndex,
    TileMap& tileMap,
    int worldWidth,
    int worldHeight,
    CollisionHandle& collisionHandle
)
{
    /*
    MovementHandle 本帧数据流：
        BehaviorIntent
            -> inputX/inputY
            -> sprinting/currentSpeed
            -> jump/velocityY
            -> wantMoveX/wantMoveY
            -> CollisionHandle 计算 allowedMoveX/allowedMoveY
            -> 写回 self.x/self.y/self.velocityY/self.onGround 等状态

    注意：
        MovementHandle 负责“想怎么动”和“把结果应用到实体”；
        CollisionHandle 负责“这个移动是否会被阻挡、最多能走多少”。
    */
    double inputX = intent.moveX;
    double inputY = intent.moveY;

    double currentSpeed = self.speed;

    bool hasMoveInput = false;

    if (inputX != 0)
    {
        hasMoveInput = true;
    }

    // 朝向是真实游戏状态，后续会影响攻击、交互、射线检测等逻辑，因此由移动层更新。
    if (inputX < -EPS)
    {
        self.setFacingDirection(LEFT);
    }
    else if (inputX > EPS)
    {
        self.setFacingDirection(RIGHT);
    }

    bool wantSprint = false;

    if (intent.wantSprint && hasMoveInput)
    {
        wantSprint = true;
    }

    if (!wantSprint)
    {
        self.sprinting = false;
    }
    else
    {
        if (!self.sprinting && self.onGround)
        {
            self.sprinting = true;
        }

        // 如果 sprinting 本来就是 true，就允许它在空中继续保持。
    }

    if (self.sprinting)
    {
        currentSpeed = self.speed * 2;
    }

    // god 模式：不受重力、不受阻挡碰撞影响，可以自由移动。
    if (self.god)
    {
        double length = sqrt(inputX * inputX + inputY * inputY);

        if (length != 0)
        {
            inputX = inputX / length;
            inputY = inputY / length;
        }

        self.x += inputX * currentSpeed;
        self.y += inputY * currentSpeed;

        collisionHandle.limitInWorld(self, worldWidth, worldHeight);
        return;
    }

    // 非 god 模式：启用重力、跳跃、碰撞。
    if (intent.wantJump && self.onGround)
    {
        self.velocityY = JUMP_SPEED;
        self.onGround = false;
        self.InAir = true;
        self.jumping = true;
    }

    // X 轴期望位移：
    //   inputX 只表示方向：-1 左，0 不动，1 右
    //   currentSpeed 是本帧速度
    //   因为当前项目把 1 tick 当作单位时间，所以：
    //   wantMoveX = inputX * currentSpeed * 1
    double wantMoveX = inputX * currentSpeed;

    double allowedMoveX = collisionHandle.getAllowedMoveX(
        self,
        wantMoveX,
        entitys,
        selfIndex,
        tileMap
    );
    if (fabs(allowedMoveX - wantMoveX) > EPS)
    {
        self.blockedByEntity = true;
        self.collisionState = true;
    }

    self.x += allowedMoveX;

    // Y 轴速度更新：
    //   速度 velocityY 每 tick 受重力影响减小
    //   velocityY -= GRAVITY
    //   wantMoveY = velocityY * 1
    // 当前项目把每帧时间简化为 1 tick，因此位移直接使用 velocityY。
    self.velocityY -= GRAVITY;

    if (self.velocityY < MAX_FALL_SPEED)
    {
        self.velocityY = MAX_FALL_SPEED;
    }

    self.onGround = false;
    self.InAir = true;

    double wantMoveY = self.velocityY;

    double allowedMoveY = collisionHandle.getAllowedMoveY(
        self,
        wantMoveY,
        entitys,
        selfIndex,
        tileMap
    );
    if (fabs(allowedMoveY - wantMoveY) > EPS)
    {
        if (wantMoveY < 0)
        {
            self.onGround = true;
            self.InAir = false;
            self.jumping = false;
        }
        else if (wantMoveY > 0)
        {
            self.blockedByEntity = true;
            self.collisionState = true;
        }

        self.velocityY = 0;
    }

    self.y += allowedMoveY;

    collisionHandle.limitInWorld(self, worldWidth, worldHeight);
}
