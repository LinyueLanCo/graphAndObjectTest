#pragma once

#include <string>
#include <vector>
#include "Controller.h"

class Entity;
class AnimationClipManager;

// 数据驱动动画规则与条件定义
struct Condition
{
    std::string paramName;
    float expectedValue;
};

struct TransitionRule
{
    std::string fromState;
    std::string toState;
    std::vector<Condition> conditions;
};

// Animator：
// 动画状态控制组件。它读取 Entity 的真实状态，并依据过渡规则表执行状态匹配。
class Animator
{
private:
    // 当前动画表现状态，用于避免每帧重复 setClip。
    std::string currentAnimState;

    // 当前 Animator 绑定的实体模板名称。
    std::string templateName;

    // 初始化时要绑定的动画表现状态，用于在第一帧绘制前设置默认 clip。
    std::string initialAnimState;

    // 本实体持有的状态过渡规则表
    std::vector<TransitionRule> transitionRules;

public:
    Animator();

    void changeAnimation(Entity& entity, const std::string& newState);
    void update(Entity& entity, BehaviorIntent intent);
    void initAnimation(Entity& entity);
    void configure(const std::string& newTemplateName, const std::string& newInitialState, const std::vector<TransitionRule>& rules);
    std::string getCurrentState() const { return currentAnimState; }
};

