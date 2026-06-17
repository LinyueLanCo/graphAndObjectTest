#include "Animator.h"
#include "Entity.h"

// 功能：初始化动画状态缓存。
Animator::Animator()
{
    currentAnimState = ANIM_COUNT;
    wasInAir = false;
    animationSetId = ANIM_SET_NONE;
    initialAnimState = ANIM_IDLE_R;
}

// 功能：按动画状态切换实体当前播放的 AnimationClip。
void Animator::changeAnimation(Entity& entity, AnimationState newState, AnimationClipManager& animationClips)
{
    if (currentAnimState == newState)
    {
        return;
    }

    // 用当前资源组和动画状态解析具体资源 ID，避免 Animator 直接绑定某个角色资源。
    AnimationId animationId = getAnimationId(animationSetId, newState);

    if (animationId == ANIM_ID_COUNT)
    {
        return;
    }

    AnimationClip clip = animationClips.getClip(animationId);

    if (clip.image == NULL)
    {
        return;
    }

    // 只有拿到有效 clip 后才写入播放器并更新当前状态缓存。
    entity.setAnimationClip(clip);
    currentAnimState = newState;
}

// 功能：读取实体真实状态并决定 idle / walk / run / jumpStart / jumpLoop / jumpEnd。
void Animator::update(Entity& entity, BehaviorIntent intent, AnimationClipManager& animationClips)
{
    if (animationSetId == ANIM_SET_CHECKPOINT)
    {
        if (currentAnimState == ANIM_CHECKPOINT_FLAG_OUT && entity.isAnimationFinished())
        {
            changeAnimation(entity, ANIM_CHECKPOINT_FLAG_IDLE, animationClips);
        }
        return;
    }

    if (animationSetId == ANIM_SET_COIN_GOLD ||
        animationSetId == ANIM_SET_COIN_SILVER ||
        animationSetId == ANIM_SET_COIN_COPPER)
    {
        return;
    }

    if (!entity.isControlled())
    {
        return;
    }

    double inputX = intent.moveX;

    bool hasMoveInput = fabs(inputX) > EPS;
    bool justLanded = wasInAir && entity.isOnGround();

    AnimationState idleState = ANIM_IDLE_L;
    AnimationState walkState = ANIM_WALK_LEFT;
    AnimationState runState = ANIM_RUN_LEFT;
    AnimationState jumpStartState = ANIM_JUMP_START_L;
    AnimationState jumpLoopState = ANIM_JUMP_LOOP_L;
    AnimationState jumpEndState = ANIM_JUMP_END_L;

    if (entity.getFacingDirection() == RIGHT)
    {
        idleState = ANIM_IDLE_R;
        walkState = ANIM_WALK_RIGHT;
        runState = ANIM_RUN_RIGHT;
        jumpStartState = ANIM_JUMP_START_R;
        jumpLoopState = ANIM_JUMP_LOOP_R;
        jumpEndState = ANIM_JUMP_END_R;
    }

    if (justLanded && !hasMoveInput)
    {
        changeAnimation(entity, jumpEndState, animationClips);
        wasInAir = entity.isInAir();
        return;
    }

    bool currentIsJumpStart =
        currentAnimState == ANIM_JUMP_START_L ||
        currentAnimState == ANIM_JUMP_START_R;

    if (entity.isInAir())
    {
        bool shouldPlayJumpStart =
            entity.isJumping() &&
            intent.wantJump &&
            !wasInAir;

        if (shouldPlayJumpStart)
        {
            changeAnimation(entity, jumpStartState, animationClips);
            wasInAir = entity.isInAir();
            return;
        }

        if (currentIsJumpStart)
        {
            if (entity.isAnimationFinished())
            {
                changeAnimation(entity, jumpLoopState, animationClips);
            }

            wasInAir = entity.isInAir();
            return;
        }

        changeAnimation(entity, jumpLoopState, animationClips);
        wasInAir = entity.isInAir();
        return;
    }

    bool currentIsJumpEnd =
        currentAnimState == ANIM_JUMP_END_L ||
        currentAnimState == ANIM_JUMP_END_R;

    if (currentIsJumpEnd && !hasMoveInput && !entity.isAnimationFinished())
    {
        wasInAir = entity.isInAir();
        return;
    }

    if (hasMoveInput)
    {
        if (entity.isSprinting())
        {
            changeAnimation(entity, runState, animationClips);
        }
        else
        {
            changeAnimation(entity, walkState, animationClips);
        }
    }
    else
    {
        changeAnimation(entity, idleState, animationClips);
    }

    wasInAir = entity.isInAir();
}

// 功能：配置 Animator 的资源组和初始状态，并重置动画状态缓存。
void Animator::configure(AnimationSetId newSetId, AnimationState newInitialState)
{
    animationSetId = newSetId;
    initialAnimState = newInitialState;
    currentAnimState = ANIM_COUNT;
    wasInAir = false;
}

// 功能：在资源加载完成后，根据初始状态绑定实体第一段动画。
void Animator::initAnimation(Entity& entity, AnimationClipManager& animationClips)
{
    if (animationSetId == ANIM_SET_NONE)
    {
        return;
    }

    changeAnimation(entity, initialAnimState, animationClips);
}
