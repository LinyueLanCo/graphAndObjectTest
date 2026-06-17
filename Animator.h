#pragma once

#include "Animation.h"
#include "Controller.h"

class Entity;

// Animator：
// 动画状态控制组件。它读取 Entity 的真实状态，决定当前应该播放哪个 AnimationState。
class Animator
{
private:
    // 当前动画表现状态，用于避免每帧重复 setClip。
    AnimationState currentAnimState;

    // 上一帧是否在空中，用于判断“刚刚落地”。
    bool wasInAir;

    // 当前 Animator 绑定的动画资源组，用于把 AnimationState 解析为具体 AnimationClip。
    AnimationSetId animationSetId;

    // 初始化时要绑定的动画表现状态，用于在第一帧绘制前设置默认 clip。
    AnimationState initialAnimState;

public:
    Animator();

    void changeAnimation(Entity& entity, AnimationState newState, AnimationClipManager& animationClips);
    void update(Entity& entity, BehaviorIntent intent, AnimationClipManager& animationClips);
    void initAnimation(Entity& entity, AnimationClipManager& animationClips);
    void configure(AnimationSetId newSetId, AnimationState newInitialState);
    AnimationState getCurrentState() const { return currentAnimState; }
};
