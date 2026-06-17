#include "Entity.h"

static int gNextEntityId = 1;

// 功能：初始化一个默认实体及其基础状态。
Entity::Entity()
{
    id = gNextEntityId++;
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
    id = gNextEntityId++;
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
}

// 功能：按初始逻辑状态和动画资源组创建实体，不再直接从构造函数加载图片路径。
Entity::Entity(
    double startX,
    double startY,
    bool isControlled,
    bool isCollidable,
    bool isBlocking,
    bool isGod,
    EntityType Type,
    AnimationSetId animationSet,
    facingDirection initialFacing,
    AnimationState initialAnim,
    bool alive
)
{
    id = gNextEntityId++;
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

    animator.configure(animationSet, initialAnim);
}

// 功能：按默认朝向和默认待机状态创建绑定动画资源组的实体。
Entity::Entity(
    double startX,
    double startY,
    bool isControlled,
    bool isCollidable,
    bool isBlocking,
    bool isGod,
    EntityType Type,
    AnimationSetId animationSet,
    bool alive
)
    : Entity(
        startX,
        startY,
        isControlled,
        isCollidable,
        isBlocking,
        isGod,
        Type,
        animationSet,
        RIGHT,
        ANIM_IDLE_R,
        alive
    )
{
}

// 功能：获取实体唯一标识 ID。
int Entity::getId() const
{
    return id;
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

// 功能：判断实体当前非循环动画是否已经播放结束。
bool Entity::isAnimationFinished()
{
    return animation.isFinished();
}

AnimationState Entity::getAnimationState() const
{
    return animator.getCurrentState();
}

// 功能：把新的动画片段绑定到实体的动画播放器。
void Entity::setAnimationClip(AnimationClip clip)
{
    animation.setClip(clip);
}

// 功能：委托实体内部 Animator 更新动画状态。
void Entity::updateAnimator(BehaviorIntent intent, AnimationClipManager& animationClips)
{
    animator.update(*this, intent, animationClips);
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
bool Entity::getIsAlive()
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
void Entity::addOverlap(int otherId, EntityType otherType)
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
void Entity::resolveOverlaps(std::vector<Entity>& allEntities, AnimationClipManager& animationClips)
{
    if (!isAlive)
    {
        return;
    }

    for (int i = 0; i < (int)currentOverlaps.size(); i++)
    {
        int otherId = currentOverlaps[i].otherEntityId;
        EntityType otherType = currentOverlaps[i].otherType;

        // 1. Player 侧重叠逻辑：只进行加分调试打印，不直接操作金币实体死亡，静待金币自毁。
        if (entityType == PLAYER)
        {
            if (otherType == COIN)
            {
                cout << "Player (ID: " << id << ") detected overlap with Coin (ID: " << otherId << ") - [Add Score Placeholder]" << endl;
            }
        }

        // 2. Coin 侧重叠逻辑：金币发现碰触 Player 后进入被收集动画，并在动画播放后自毁
        if (entityType == COIN)
        {
            if (otherType == PLAYER)
            {
                PlaySoundW(
                    _T("assets\\sound\\entities\\item\\coin_pickup.wav"),
                    NULL,
                    SND_ASYNC | SND_NOSTOP
                );
                collidable = false; // 禁用后续碰撞，避免重复响应
                animator.changeAnimation(*this, ANIM_COLLECTED, animationClips);
                std::cout << "Coin (ID: " << id << ") triggered collected animation." << std::endl;
            }
        }

        // 3. Checkpoint 侧重叠逻辑：玩家碰触后从 No Flag 切换为 Flag Out 播放升旗动画
        if (entityType == CHECKPOINT)
        {
            if (otherType == PLAYER)
            {
                if (getAnimationState() != ANIM_CHECKPOINT_FLAG_OUT && getAnimationState() != ANIM_CHECKPOINT_FLAG_IDLE)
                {
                    animator.changeAnimation(*this, ANIM_CHECKPOINT_FLAG_OUT, animationClips);
                    std::cout << "Checkpoint (ID: " << id << ") activated! Flag raising." << std::endl;
                }
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

// 功能：设置实体碰撞盒相对实体中心点的偏移。
void Entity::setCollisionBoxOffset(double offsetX, double offsetY)
{
    collisionBox.setOffset(offsetX, offsetY);
}

// 功能：设置实体碰撞盒缩放比例。
void Entity::setCollisionScale(double scaleX, double scaleY)
{
    collisionBox.setScale(scaleX, scaleY);
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
    if (entityType == COIN && getAnimationState() == ANIM_COLLECTED && isAnimationFinished())
    {
        killEntity();
        std::cout << "Coin (ID: " << id << ") destroyed after collected animation finished." << std::endl;
    }
}

// 功能：设置实体 sprite 绘制缩放和偏移。
void Entity::setSpriteTransform(double scaleX, double scaleY, double offsetX, double offsetY)
{
    renderSprite.setTransform(scaleX, scaleY, offsetX, offsetY);
    syncRenderSpriteWorldDrawData();
}

// 功能：设置实体动画播放速度。
void Entity::setAnimationSpeed(int speed)
{
    animation.setSpeed(speed);
}

// 功能：让实体内部 Animator 根据初始状态绑定动画，并同步第一帧 sprite 和碰撞盒尺寸。
void Entity::initAnimationFromAnimator(AnimationClipManager& animationClips)
{
    animator.initAnimation(*this, animationClips);

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

