#include "Animator.h"
#include "Entity.h"
#include <cmath>
#include <iostream>

// 功能：初始化动画状态缓存。
Animator::Animator()
{
    currentAnimState = "";
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

// 功能：读取实体真实状态，在参数字典中更新当前帧的值，然后根据配置的过渡规则表比对并切换状态。
void Animator::update(Entity& entity, BehaviorIntent intent)
{
    // 如果没有配置任何过渡规则（比如静态物体或下落平台），直接返回
    if (transitionRules.empty())
    {
        return;
    }

    // 1. 每帧刷新实体的动画参数字典
    entity.animParams["inAir"] = entity.isInAir() ? 1.0f : 0.0f;
    entity.animParams["wasInAir"] = entity.lastInAirState ? 1.0f : 0.0f;
    entity.animParams["onGround"] = entity.isOnGround() ? 1.0f : 0.0f;
    entity.animParams["moving"] = (fabs(intent.moveX) > 1e-6) ? 1.0f : 0.0f;
    entity.animParams["sprinting"] = entity.isSprinting() ? 1.0f : 0.0f;
    entity.animParams["isJumping"] = entity.isJumping() ? 1.0f : 0.0f;
    entity.animParams["facing"] = (entity.getFacingDirection() == RIGHT) ? 1.0f : -1.0f;
    entity.animParams["animFinished"] = entity.isAnimationFinished() ? 1.0f : 0.0f;
    entity.animParams["platformState"] = (float)entity.platformState;
    entity.animParams["shouldPlayJumpStart"] = (entity.isJumping() && intent.wantJump && !entity.lastInAirState) ? 1.0f : 0.0f;
    entity.animParams["justLanded"] = (entity.lastInAirState && entity.isOnGround()) ? 1.0f : 0.0f;
    entity.animParams["isJumpStart"] = (currentAnimState == "jump_start_r" || currentAnimState == "jump_start_l") ? 1.0f : 0.0f;
    entity.animParams["isJumpEnd"] = (currentAnimState == "jump_end_r" || currentAnimState == "jump_end_l") ? 1.0f : 0.0f;

    std::string nextState = currentAnimState;

    // 2. 顺序遍历过渡规则表进行比对
    for (const auto& rule : transitionRules)
    {
        // 匹配起始状态：当前动画状态，或通用匹配 "any"
        if (rule.fromState == "any" || rule.fromState == currentAnimState)
        {
            bool allConditionsMet = true;
            for (const auto& cond : rule.conditions)
            {
                auto it = entity.animParams.find(cond.paramName);
                float val = (it != entity.animParams.end()) ? it->second : 0.0f;
                if (val != cond.expectedValue)
                {
                    allConditionsMet = false;
                    break;
                }
            }

            if (allConditionsMet)
            {
                nextState = rule.toState;
                break; // 顺序比对，匹配到第一条规则即应用并结束
            }
        }
    }

    // 3. 执行状态变更
    if (nextState != currentAnimState)
    {
        changeAnimation(entity, nextState);
    }
}

// 功能：配置 Animator 的实体模板、初始状态和过渡规则表，并重置动画状态缓存。
void Animator::configure(const std::string& newTemplateName, const std::string& newInitialState, const std::vector<TransitionRule>& rules)
{
    templateName = newTemplateName;
    initialAnimState = newInitialState;
    transitionRules = rules;
    currentAnimState = "";
}

// 功能：执行初始动画绑定。
void Animator::initAnimation(Entity& entity)
{
    if (templateName.empty())
    {
        return;
    }

    changeAnimation(entity, initialAnimState);
}
