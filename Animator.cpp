#include "Animator.h"
#include "Entity.h"
#include <cmath>

// 功能：初始化动画状态缓存。
Animator::Animator()
{
    currentAnimState = "";
    wasInAir = false;
    templateName = "";
    initialAnimState = "idle";
}

// 功能：按动画状态切换实体当前播放的 AnimationClip。
void Animator::changeAnimation(Entity& entity, const std::string& newState)
{
    if (currentAnimState == newState)
    {
        return;
    }

    AnimationClip clip = entity.getClipForState(newState);

    if (clip.image == NULL)
    {
        return;
    }

    // 只有拿到有效 clip 后才写入播放器并更新当前状态缓存。
    entity.setAnimationClip(clip);
    currentAnimState = newState;
}

// 功能：读取实体真实状态并决定 idle / walk / run / jumpStart / jumpLoop / jumpEnd。
void Animator::update(Entity& entity, BehaviorIntent intent)
{
    if (templateName == "Endpoint")
    {
        // 改为判断当前是否处于 pressed 状态，且该非循环动画已经播放结束
        if (currentAnimState == "pressed" && entity.isAnimationFinished())
        {
            changeAnimation(entity, "collected");
        }
        return;
    }

    if (templateName == "Checkpoint")
    {
        if (currentAnimState == "flag_out" && entity.isAnimationFinished())
        {
            changeAnimation(entity, "flag_idle");
        }
        return;
    }

    if (templateName == "CoinGold" ||
        templateName == "CoinSilver" ||
        templateName == "CoinCopper" ||
        templateName == "Apple" ||
        templateName == "Banana" ||
        templateName == "Cherry")
    {
        return;
    }

    if (!entity.isControlled())
    {
        return;
    }

    double inputX = intent.moveX;

    bool hasMoveInput = fabs(inputX) > 1e-6; // EPS = 1e-6
    bool justLanded = wasInAir && entity.isOnGround();

    std::string idleState = "idle_l";
    std::string walkState = "walk_l";
    std::string runState = "run_l";
    std::string jumpStartState = "jump_start_l";
    std::string jumpLoopState = "jump_loop_l";
    std::string jumpEndState = "jump_end_l";

    if (entity.getFacingDirection() == RIGHT)
    {
        idleState = "idle_r";
        walkState = "walk_r";
        runState = "run_r";
        jumpStartState = "jump_start_r";
        jumpLoopState = "jump_loop_r";
        jumpEndState = "jump_end_r";
    }

    if (justLanded && !hasMoveInput)
    {
        changeAnimation(entity, jumpEndState);
        wasInAir = entity.isInAir();
        return;
    }

    bool currentIsJumpStart =
        currentAnimState == "jump_start_l" ||
        currentAnimState == "jump_start_r";

    if (entity.isInAir())
    {
        bool shouldPlayJumpStart =
            entity.isJumping() &&
            intent.wantJump &&
            !wasInAir;

        if (shouldPlayJumpStart)
        {
            changeAnimation(entity, jumpStartState);
            wasInAir = entity.isInAir();
            return;
        }

        if (currentIsJumpStart)
        {
            if (entity.isAnimationFinished())
            {
                changeAnimation(entity, jumpLoopState);
            }

            wasInAir = entity.isInAir();
            return;
        }

        changeAnimation(entity, jumpLoopState);
        wasInAir = entity.isInAir();
        return;
    }

    bool currentIsJumpEnd =
        currentAnimState == "jump_end_l" ||
        currentAnimState == "jump_end_r";

    if (currentIsJumpEnd && !hasMoveInput && !entity.isAnimationFinished())
    {
        wasInAir = entity.isInAir();
        return;
    }

    if (hasMoveInput)
    {
        if (entity.isSprinting())
        {
            changeAnimation(entity, runState);
        }
        else
        {
            changeAnimation(entity, walkState);
        }
    }
    else
    {
        changeAnimation(entity, idleState);
    }

    wasInAir = entity.isInAir();
}

// 功能：配置 Animator 的实体模板和初始状态，并重置动画状态缓存。
void Animator::configure(const std::string& newTemplateName, const std::string& newInitialState)
{
    templateName = newTemplateName;
    initialAnimState = newInitialState;
    currentAnimState = "";
    wasInAir = false;
}

void Animator::initAnimation(Entity& entity)
{
    if (templateName.empty())
    {
        return;
    }

    changeAnimation(entity, initialAnimState);
}
