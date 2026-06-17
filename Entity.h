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
class EntityManager;

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

    bool getIsAlive() const;
    void setIsAlive(bool value);
    void killEntity();

    // 重置大复活术：擦除槽位中实体上辈子的各种状态残留，直接将新的参数重新装载到当前对象上
    // 参数意义：
    //   entityId: 赋予实体的新 ID 名字
    //   startX, startY: 新生出的世界坐标位置
    //   isControlled: 新生后是否允许被键盘控制
    //   isCollidable: 新生后是否能够重叠交互
    //   isBlocking: 新生后是否具有阻挡墙壁体积
    //   isGod: 是否开启上帝无敌模式
    //   Type: 实体类型标识（PLAYER, COIN 等）
    //   animationSet: 绑定的动画包包 ID
    //   alive: 初始生存标记
    void reset(
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

    double getX();
    double getY();

    sprite& getRenderSprite();
    const sprite& getSprite() const;

    void setOverlapping(bool value);
    void addOverlap(const std::string& otherId, EntityType otherType);
    const std::vector<OverlapInfo>& getCurrentOverlaps() const;

    // 实体自治逻辑：让实体自己去处理本帧记录在 currentOverlaps 里的碰撞对象并执行动作（吃硬币、升旗帜等）
    // 参数意义：
    //   entityManager: 统一实体管理器引用，利用 O(1) 字典查找对方实体
    //   animationClips: 动画片段管理器，用于切换金币“被吃动画”或者旗帜“升旗动画”
    void resolveOverlaps(EntityManager& entityManager, AnimationClipManager& animationClips);
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

