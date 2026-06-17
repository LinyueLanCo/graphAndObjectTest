#pragma once

#include <string>
#include "Animator.h"
#include "Animation.h"
#include "Collision.h"
#include "Config.h"
#include "Controller.h"
#include "GameTypes.h"
#include "Sprite.h"

class CollisionHandle;
class MovementHandle;

// Entity：
// 实体数据容器 + 组件持有者，保存真实游戏状态并提供碰撞、动画、绘制接口。
class Entity
{
    friend class MovementHandle;
    friend class CollisionHandle;
    friend class Animator;

private:
    std::string id;
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
        std::string entityId,
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
        bool alive = 1
    );
    Entity(
        std::string entityId,
        double startX,
        double startY,
        bool isControlled,
        bool isCollidable,
        bool isBlocking,
        bool isGod,
        EntityType Type,
        AnimationSetId animationSet,
        bool alive = 1
    );

    std::string getId() const;
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
    AnimationState getAnimationState() const;
    void setAnimationClip(AnimationClip clip);
    void updateAnimator(BehaviorIntent intent, AnimationClipManager& animationClips);

    bool hasCollisionState();
    bool isBlockedByEntity();
    bool isBlockedByWorld();

    bool getIsAlive();
    void setIsAlive(bool value);
    void killEntity();

    double getX();
    double getY();

    sprite& getRenderSprite();
    const sprite& getSprite() const;

    void setOverlapping(bool value);
    void addOverlap(const std::string& otherId, EntityType otherType);
    const std::vector<OverlapInfo>& getCurrentOverlaps() const;
    void resolveOverlaps(std::vector<Entity>& allEntities, AnimationClipManager& animationClips);
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
    void initAnimationFromAnimator(AnimationClipManager& animationClips);
};

