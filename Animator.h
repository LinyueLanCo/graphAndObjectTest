#pragma once

#include <string>
#include "Controller.h"

class Entity;
class AnimationClipManager;

// Animator：
// 动画状态控制组件。它读取 Entity 的真实状态，决定当前应该播放哪个动画状态。
class Animator
{
private:
    // 当前动画表现状态，用于避免每帧重复 setClip。
    std::string currentAnimState;

    // 上一帧是否在空中，用于判断“刚刚落地”。
    bool wasInAir;

    // 当前 Animator 绑定的实体模板名称，用于在运行时区分特定类型的实体行为分支。
    std::string templateName;

    // 初始化时要绑定的动画表现状态，用于在第一帧绘制前设置默认 clip。
    std::string initialAnimState;

public:
    Animator();

    void changeAnimation(Entity& entity, const std::string& newState);
    void update(Entity& entity, BehaviorIntent intent);
    void initAnimation(Entity& entity);
    void configure(const std::string& newTemplateName, const std::string& newInitialState);
    std::string getCurrentState() const { return currentAnimState; }
};

