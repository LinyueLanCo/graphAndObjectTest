#include "Entity.h"
#include "EntityManager.h"

static int gNextEntityId = 1;

// 功能：初始化一个默认实体及其基础状态。
Entity::Entity()
{
    instanceId = INVALID_ENTITY_ID;
    name = "Entity_" + std::to_string(gNextEntityId++);
    x = 0;
    y = 0;
    speed = 5;
    velocityY = 0;

    controlled = false;
    collidable = false;
    blocking = false;
    god = true;

    overlapping = false;
    collisionState = false;

    onGround = false;
    sprinting = false;
    InAir = false;
    jumping = false;
    blockedByEntity = false;
    blockedByWorld = false;

    currentFacingDirection = LEFT;

    entityType = DEFAULT;
    isAlive = 1;

    lastCollisionState = false;
    lastGroundState = false;
    lastSprintState = false;
    lastInAirState = false;
    lastJumpingState = false;
    lastAliveState = true;
    flagActivatedJustNow = false;

    platformState = 0;
    platformTimer = 0.0;
    animParams.clear();
}

// 功能：按资源路径和初始属性创建一个可用实体。
Entity::Entity(
    const TCHAR* imagePath,
    double startX,
    double startY,
    bool isControlled,
    bool isCollidable,
    bool isBlocking,
    bool isGod,
    EntityType Type,
    int frameCount,
    bool alive
)
{
    instanceId = INVALID_ENTITY_ID;
    name = "Entity_" + std::to_string(gNextEntityId++);
    animation.load(imagePath, frameCount);
    animation.setSpeed(4);
    animation.setLoop(true);

    x = startX;
    y = startY;

    speed = 5;
    velocityY = 0;

    controlled = isControlled;
    collidable = isCollidable;
    blocking = isBlocking;
    god = isGod;

    overlapping = false;
    collisionState = false;

    onGround = false;
    sprinting = false;
    InAir = false;
    jumping = false;
    blockedByEntity = false;
    blockedByWorld = false;

    int imgW = animation.getFrameWidth();
    int imgH = animation.getFrameHeight();

    collisionBox.setBaseSize(imgW, imgH);

    currentFacingDirection = LEFT;

    entityType = Type;
    isAlive = alive;

    lastCollisionState = false;
    lastGroundState = false;
    lastSprintState = false;
    lastInAirState = false;
    lastJumpingState = false;
    lastAliveState = alive;
    flagActivatedJustNow = false;

    platformState = 0;
    platformTimer = 0.0;
    animParams.clear();
}

// 功能：按初始逻辑状态和动画资源模板创建实体，不再直接从构造函数加载图片路径。
Entity::Entity(
    EntityID instanceId,
    const std::string& entityName,
    double startX,
    double startY,
    bool isControlled,
    bool isCollidable,
    bool isBlocking,
    bool isGod,
    EntityType Type,
    const std::string& tempName,
    facingDirection initialFacing,
    const std::string& initialAnim,
    bool alive
)
{
    this->instanceId = instanceId;
    name = entityName;
    x = startX;
    y = startY;

    speed = 5;
    velocityY = 0;

    controlled = isControlled;
    collidable = isCollidable;
    blocking = isBlocking;
    god = isGod;

    overlapping = false;
    collisionState = false;

    onGround = false;
    sprinting = false;
    InAir = false;
    jumping = false;
    blockedByEntity = false;
    blockedByWorld = false;

    currentFacingDirection = initialFacing;

    entityType = Type;
    isAlive = alive;
    templateName = tempName;

    animator.configure(tempName, initialAnim, {});

    lastCollisionState = false;
    lastGroundState = false;
    lastSprintState = false;
    lastInAirState = false;
    lastJumpingState = false;
    lastAliveState = alive;
    flagActivatedJustNow = false;

    platformState = 0;
    platformTimer = 0.0;
    animParams.clear();
}

// 功能：按默认朝向和默认待机状态创建绑定动画资源模板的实体。
Entity::Entity(
    EntityID instanceId,
    const std::string& entityName,
    double startX,
    double startY,
    bool isControlled,
    bool isCollidable,
    bool isBlocking,
    bool isGod,
    EntityType Type,
    const std::string& tempName,
    bool alive
)
    : Entity(
        instanceId,
        entityName,
        startX,
        startY,
        isControlled,
        isCollidable,
        isBlocking,
        isGod,
        Type,
        tempName,
        RIGHT,
        "idle",
        alive
    )
{
}

// 功能：重置实体的所有属性以重用槽位。
void Entity::reset(
    EntityID instanceId,
    const std::string& entityName,
    double startX,
    double startY,
    bool isControlled,
    bool isCollidable,
    bool isBlocking,
    bool isGod,
    EntityType Type,
    const std::string& tempName,
    bool alive
)
{
    this->instanceId = instanceId;
    name = entityName;
    x = startX;
    y = startY;
    controlled = isControlled;
    collidable = isCollidable;
    blocking = isBlocking;
    god = isGod;
    entityType = Type;
    isAlive = alive;
    templateName = tempName;

    speed = 5;
    velocityY = 0;
    overlapping = false;
    collisionState = false;
    onGround = false;
    sprinting = false;
    InAir = false;
    jumping = false;
    blockedByEntity = false;
    blockedByWorld = false;
    currentFacingDirection = RIGHT;

    currentOverlaps.clear();

    animator.configure(tempName, "idle", {});

    lastCollisionState = false;
    lastGroundState = false;
    lastSprintState = false;
    lastInAirState = false;
    lastJumpingState = false;
    lastAliveState = alive;
    flagActivatedJustNow = false;

    platformState = 0;
    platformTimer = 0.0;
    animParams.clear();
}

// 功能：获取实体唯一标识 ID。
EntityID Entity::getId() const
{
    return instanceId;
}

// 功能：获取实体类型标签。
EntityType Entity::getEntityType()
{
    return entityType;
}

// 功能：判断实体是否参与重叠事件检测。
bool Entity::isCollidable()
{
    return collidable;
}

// 功能：判断实体是否作为阻挡物参与移动修正。
bool Entity::isBlocking()
{
    return blocking;
}

// 功能：判断实体是否处于 god 模式。
bool Entity::isGod()
{
    return god;
}

// 功能：判断实体当前是否站在地面或平台上。
bool Entity::isOnGround()
{
    return onGround;
}

// 功能：判断实体当前是否处于空中。
bool Entity::isInAir()
{
    return InAir;
}

// 功能：判断实体当前是否正在冲刺。
bool Entity::isSprinting()
{
    return sprinting;
}

// 功能：判断实体当前是否处于跳跃状态。
bool Entity::isJumping()
{
    return jumping;
}

// 功能：判断实体是否由玩家输入控制。
bool Entity::isControlled()
{
    return controlled;
}

void Entity::setControlled(bool value)
{
    controlled = value;
}

// 功能：返回实体当前朝向。
facingDirection Entity::getFacingDirection()
{
    return currentFacingDirection;
}

// 功能：设置实体当前朝向。
void Entity::setFacingDirection(facingDirection direction)
{
    currentFacingDirection = direction;
}



// 功能：委托实体内部 Animator 更新动画状态。
void Entity::updateAnimator(BehaviorIntent intent)
{
    animator.update(*this, intent);
}

// 功能：判断实体本帧是否处于碰撞或重叠反馈状态。
bool Entity::hasCollisionState()
{
    return collisionState;
}

// 功能：判断实体本帧是否被其它实体阻挡。
bool Entity::isBlockedByEntity()
{
    return blockedByEntity;
}

// 功能：判断实体本帧是否被世界边界阻挡。
bool Entity::isBlockedByWorld()
{
    return blockedByWorld;
}

// 功能：获取实体当前是否存活。
bool Entity::getIsAlive() const
{
    return isAlive;
}

// 功能：设置实体存活状态。
void Entity::setIsAlive(bool value)
{
    isAlive = value;
}

// 功能：将实体标记为死亡。
void Entity::killEntity()
{
    isAlive = 0;
}

// 功能：获取实体中心点的世界 X 坐标。
double Entity::getX()
{
    return x;
}

// 功能：获取实体中心点的世界 Y 坐标。
double Entity::getY()
{
    return y;
}

// 功能：获取实体当前用于渲染的 sprite 可写接口。
sprite& Entity::getRenderSprite()
{
    return renderSprite;
}

// 功能：获取实体当前用于渲染的 sprite 只读接口。
const sprite& Entity::getSprite() const
{
    return renderSprite;
}

// 功能：设置实体本帧重叠状态并触发碰撞反馈显示。
void Entity::setOverlapping(bool value)
{
    overlapping = value;

    if (value)
    {
        collisionState = true;
    }
}

// 功能：向实体中追加一条重叠对象记录。
void Entity::addOverlap(EntityID otherId, EntityType otherType)
{
    OverlapInfo info;
    info.otherEntityId = otherId;
    info.otherType = otherType;
    currentOverlaps.push_back(info);
}

// 功能：获取实体当前的重叠列表只读引用。
const std::vector<OverlapInfo>& Entity::getCurrentOverlaps() const
{
    return currentOverlaps;
}

// 功能：实体自治处理自身重叠反应逻辑。
// 核心自治反应功能：实体自己去读取本帧被塞进来的重叠记录，自己决定干嘛。
// 
// 参数意义：
//   entityManager: 统一实体管理器引用，利用 O(1) 字典查找对方实体
//   animationClips: 动画片段管理器，用于切换金币被吃动画或升旗动画
void Entity::resolveOverlaps(EntityManager& entityManager)
{
    // 如果我已经挂了，那就没必要做出任何碰撞反馈了
    if (!isAlive)
    {
        return;
    }

    // 循环遍历我这帧碰到的所有对象
    for (int i = 0; i < (int)currentOverlaps.size(); i++)
    {
        EntityID otherId = currentOverlaps[i].otherEntityId;
        EntityType otherType = currentOverlaps[i].otherType;

        // 1. 如果我是玩家操控的主角：
        //    我们只进行加分日志打印，具体消灭金币的事情交给金币自己去判定和自毁。
        if (controlled)
        {
            if (otherType == COIN)
            {
                Entity* otherEntity = entityManager.getEntity(otherId);
                if (otherEntity)
                {
                    cout << "主角 [" << name << "] (ID: " << instanceId << ") 碰到了金币 [" << otherEntity->getName() << "] (ID: " << otherId << ") - [准备执行加分]" << endl;
                }
            }
        }

        // 2. 如果我是可拾取物（COIN）：
        if (entityType == COIN)
        {
            Entity* otherEntity = entityManager.getEntity(otherId);

            if (otherEntity != nullptr && otherEntity->isControlled())
            {
                // 核心防呆：立刻关闭碰撞
                collidable = false;

                // 通用调试日志：直接输出本实体的具体 ID
                std::cout << "玩家拾取了物品: [" << name << "] (ID: " << instanceId << ")" << std::endl;

                // --- 特判区 ---
                // 如果你需要对某些特殊物品进行特判逻辑，可以直接根据 ID 判定：
                if (name == "SpecialGoldApple")
                {
                    // 比如吃到了特殊的黄金苹果，触发回满血或者无敌逻辑
                    std::cout << "【特殊事件】吃到了黄金苹果，主角获得无敌状态！" << std::endl;
                    // otherEntity->setGod(true); 等等
                }
                else if (name.rfind("Apple", 0) == 0) // 以 "Apple" 开头的实体
                {
                    // 普通苹果的行为
                }

                // 播放收集音效并切换至消失爆裂动画
                PlaySoundW(
                    _T("assets\\sound\\entities\\item\\coin_pickup.wav"),
                    NULL,
                    SND_ASYNC | SND_NOSTOP
                );
                animator.changeAnimation(*this, "collected");
            }
        }
        // 3. 如果我是旗杆（CHECKPOINT）：
        //    一旦玩家操控的角色碰到了我，只要我还没升过旗，我就开始切换升旗动画。
        if (entityType == CHECKPOINT)
        {
            Entity* otherEntity = entityManager.getEntity(otherId);

            // 如果确实碰到了主角，并且我还没升过旗（不处于 FLAG_OUT 和 FLAG_IDLE 状态）
            if (otherEntity != nullptr && otherEntity->isControlled())
            {
                if (animator.getCurrentState() != "flag_out" && animator.getCurrentState() != "flag_idle")
                {
                    // 切换到升旗动画
                    animator.changeAnimation(*this, "flag_out");
                    std::cout << "旗杆 [" << name << "] (ID: " << instanceId << ") 被玩家触碰，开始升旗！" << std::endl;
                }
            }
        }
        // 4. 如果我是终点（ENDPOINT）：
        if (entityType == ENDPOINT)
        {
            Entity* otherEntity = entityManager.getEntity(otherId);

            if (otherEntity != nullptr && otherEntity->isControlled())
            {
                if (animator.getCurrentState() != "pressed" && animator.getCurrentState() != "collected")
                {
                    collidable = false;
                    animator.changeAnimation(*this, "pressed");
                }
                std::cout << "玩家触碰到了终点 [" << name << "] (ID: " << instanceId << ")" << std::endl;
                // 这里可以添加过关逻辑，比如切换到下一关或者显示胜利界面
            }
        }
    }
}

// 功能：直接设置实体碰撞反馈状态。
void Entity::setCollisionState(bool value)
{
    collisionState = value;
}

// 功能：清理实体每帧临时状态。
void Entity::clearFrameState()
{
    overlapping = false;
    collisionState = false;

    blockedByEntity = false;
    blockedByWorld = false;

    currentOverlaps.clear();
    // onGround 是物理状态，不是单帧显示状态，会在移动阶段重新判断。
}

// 功能：根据指定中心点计算实体碰撞盒的世界坐标范围。
RectBox Entity::getWorldCollisionBoxAt(double testX, double testY)
{
    return collisionBox.toWorldBox(testX, testY);
}

// 功能：获取实体当前位置下的世界碰撞盒。
RectBox Entity::getWorldCollisionBox()
{
    return getWorldCollisionBoxAt(x, y);
}

// 功能：设置实体碰撞盒原始尺寸。
void Entity::setCollisionBoxSize(double width, double height)
{
    collisionBox.setBaseSize(width, height);
}



// 功能：根据实体当前位置和 sprite 自身变换，补全当前帧的世界绘制数据。
void Entity::syncRenderSpriteWorldDrawData()
{
    // 用源帧尺寸乘 sprite 缩放，得到当前帧在世界坐标里的绘制宽高。
    double worldDrawW = renderSprite.srcW * renderSprite.scaleX;
    double worldDrawH = renderSprite.srcH * renderSprite.scaleY;

    // 用实体中心点加 sprite 偏移，得到 sprite 本帧的世界中心点。
    double worldCenterX = x + renderSprite.offsetX;
    double worldCenterY = y + renderSprite.offsetY;

    renderSprite.setWorldDrawData(
        worldCenterX,
        worldCenterY,
        worldDrawW,
        worldDrawH
    );
}

// 功能：推进实体当前动画播放器，并把当前帧同步写入 renderSprite。
void Entity::updateAnimatedSprite()
{
    animation.update();
    animation.writeCurrentFrameTo(renderSprite);
    syncRenderSpriteWorldDrawData();

    // 如果是金币且播放完了收集爆裂动画，则将其真正自毁
    if (entityType == COIN && animator.getCurrentState() == "collected" && animation.isFinished())
    {
        killEntity();
        std::cout << "Coin [" << name << "] (ID: " << instanceId << ") destroyed after collected animation finished." << std::endl;
    }

    // 如果是旗帜且播放完了升旗动画，标记以动态生成金币
    if (entityType == CHECKPOINT && animator.getCurrentState() == "flag_out" && animation.isFinished())
    {
        flagActivatedJustNow = true;
    }

    // 如果是终点且播放完了收集爆裂动画，则将其真正自毁
    if (entityType == ENDPOINT && animator.getCurrentState() == "collected" && animation.isFinished())
    {
        killEntity();
        std::cout << "Endpoint [" << name << "] (ID: " << instanceId << ") collected and processed." << std::endl;
    }
}

// 功能：下落平台每帧的晃动与坠落逻辑
void Entity::updateFallingPlatform(EntityManager& entityManager)
{
    if (platformState == 0) // STATIC
    {
        RectBox myBox = getWorldCollisionBox();
        const auto& activeIndices = entityManager.getActiveIndices();
        auto& entities = entityManager.getEntities();

        for (size_t idx : activeIndices)
        {
            Entity& other = entities[idx];
            if (other.getEntityType() == PLAYER && other.getIsAlive())
            {
                RectBox playerBox = other.getWorldCollisionBox();
                bool xOverlap = (playerBox.left < myBox.right && playerBox.right > myBox.left);
                bool onTop = (fabs(playerBox.bottom - myBox.top) <= 1.0);

                if (xOverlap && onTop && other.isOnGround())
                {
                    platformState = 1; // 开启摇晃
                    platformTimer = 90; // 摇晃 90 帧 (约 1.5 秒)
                    break;
                }
            }
        }
    }
    else if (platformState == 1) // SHAKING
    {
        platformTimer -= 1.0;
        double shakeAmplitude = 4.0;
        renderSprite.offsetX = ((int)platformTimer % 4 < 2) ? shakeAmplitude : -shakeAmplitude;
        syncRenderSpriteWorldDrawData();

        if (platformTimer <= 0.0)
        {
            platformState = 2; // 下落
            renderSprite.offsetX = 0.0;
            renderSprite.zIndex = 200; // 提到最前景
            blocking = false; // 取消阻挡
            velocityY = 0.0;
            syncRenderSpriteWorldDrawData();
        }
    }
    else if (platformState == 2) // FALLING
    {
        velocityY -= 0.5;
        if (velocityY < -15.0)
        {
            velocityY = -15.0;
        }
        y += velocityY;
        syncRenderSpriteWorldDrawData();

        if (y < -200.0)
        {
            killEntity();
            std::cout << "Falling platform [" << name << "] (ID: " << instanceId << ") fell out of world." << std::endl;
        }
    }
}



// 功能：让实体内部 Animator 根据初始状态绑定动画，并同步第一帧 sprite 和碰撞盒尺寸。
void Entity::initAnimationFromAnimator()
{
    animator.initAnimation(*this);

    animation.writeCurrentFrameTo(renderSprite);
    syncRenderSpriteWorldDrawData();

    if (animation.getFrameWidth() > 0 && animation.getFrameHeight() > 0)
    {
        collisionBox.setBaseSize(
            animation.getFrameWidth(),
            animation.getFrameHeight()
        );
    }
}

// 功能：从模板映射初始化本实体的所有状态动画片段缓存。
void Entity::initAnimations(
    const std::string& tempName,
    const std::string& initialAnim,
    facingDirection initialFacing,
    const std::unordered_map<std::string, std::string>& stateToClipName,
    const std::vector<TransitionRule>& rules,
    AnimationClipManager& animClips
)
{
    templateName = tempName;
    currentFacingDirection = initialFacing;
    animator.configure(tempName, initialAnim, rules);

    myClips.clear();
    for (const auto& pair : stateToClipName)
    {
        myClips[pair.first] = animClips.getClip(pair.second);
    }
}

// 功能：根据状态名称获取本实体缓存在本地的动画片段。
AnimationClip Entity::getClipForState(const std::string& state) const
{
    auto it = myClips.find(state);
    if (it != myClips.end())
    {
        return it->second;
    }
    return AnimationClip();
}


