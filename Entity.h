#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Animator.h"
#include "Collision.h"
#include "Config.h"
#include "Controller.h"
#include "GameTypes.h"
#include "Sprite.h"
#include "AnimatedSprite.h"

class CollisionHandle;
class MovementHandle;
class EntityManager;
class AnimationClipManager;

// Entity：
// 实体数据容器 + 组件持有者，保存真实游戏状态并提供碰撞、动画、绘制接口。
class Entity
{
    friend class MovementHandle;
    friend class CollisionHandle;
    friend class Animator;

private:
    EntityID instanceId;
    std::string name;
    std::vector<OverlapInfo> currentOverlaps;

    animatedSprite animation;
    sprite renderSprite;

    double x;
    double y;

    double speed;
    double velocityY;

    bool controlled;
    bool collidable;
    bool blocking;
    bool god;

    bool overlapping;
    bool collisionState;
    bool InAir;
    bool onGround;
    bool sprinting;
    bool jumping;
    bool blockedByEntity;
    bool blockedByWorld;

    EntityType entityType;
    bool isAlive;

    CollisionBox collisionBox;
    facingDirection currentFacingDirection;
    Animator animator;

    // 数据驱动动画扩展：当前实体所使用的模板名以及本地缓存的状态-动画片段池
    std::string templateName;
    std::unordered_map<std::string, AnimationClip> myClips;

public:
    // State cache for transition logging (formerly in Level)
    bool lastCollisionState;
    bool lastGroundState;
    bool lastSprintState;
    bool lastInAirState;
    bool lastJumpingState;
    bool lastAliveState;

    // 动态生成触发标记（在旗帜等升旗动画完毕时触发）
    bool flagActivatedJustNow;

public:
    Entity();
    Entity(
        const TCHAR* imagePath,
        double startX,
        double startY,
        bool isControlled,
        bool isCollidable,
        bool isBlocking,
        bool isGod,
        EntityType Type = DEFAULT,
        int frameCount = 1,
        bool alive = 1
    );
    Entity(
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
        bool alive = true
    );
    Entity(
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
        bool alive = true
    );

    EntityID getId() const;
    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
    std::string getTemplateName() const { return templateName; }
    EntityType getEntityType();
    bool isCollidable();
    bool isBlocking();
    bool isGod();
    bool isOnGround();
    bool isInAir();
    bool isSprinting();
    bool isJumping();
    bool isControlled();
    void setControlled(bool value);

    facingDirection getFacingDirection();
    void setFacingDirection(facingDirection direction);

    bool isAnimationFinished();
    std::string getAnimationState() const;
    void setAnimationClip(AnimationClip clip);
    void updateAnimator(BehaviorIntent intent);

    bool hasCollisionState();
    bool isBlockedByEntity();
    bool isBlockedByWorld();

    bool getIsAlive() const;
    void setIsAlive(bool value);
    void killEntity();

    // 重置大复活术：擦除槽位中实体上辈子的各种状态残留，直接将新的参数重新装载到当前对象上
    void reset(
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
        bool alive = true
    );

    double getX();
    double getY();

    sprite& getRenderSprite();
    const sprite& getSprite() const;

    void setOverlapping(bool value);
    void addOverlap(EntityID otherId, EntityType otherType);
    const std::vector<OverlapInfo>& getCurrentOverlaps() const;

    // 实体自治逻辑：让实体自己去处理本帧记录在 currentOverlaps 里的碰撞对象并执行动作
    void resolveOverlaps(EntityManager& entityManager);
    void setCollisionState(bool value);
    void clearFrameState();

    RectBox getWorldCollisionBoxAt(double testX, double testY);
    RectBox getWorldCollisionBox();

    void setCollisionBoxSize(double width, double height);
    void setCollisionBoxOffset(double offsetX, double offsetY);
    void setCollisionScale(double scaleX, double scaleY);

    void syncRenderSpriteWorldDrawData();
    void updateAnimatedSprite();
    void setSpriteTransform(double scaleX, double scaleY, double offsetX, double offsetY);
    void setAnimationSpeed(int speed);
    void initAnimationFromAnimator();

    // 初始化本实体的所有状态动画片段缓存
    void initAnimations(
        const std::string& tempName,
        const std::string& initialAnim,
        facingDirection initialFacing,
        const std::unordered_map<std::string, std::string>& stateToClipName,
        AnimationClipManager& animClips
    );
    AnimationClip getClipForState(const std::string& state) const;
};


