#pragma once

#include "Input.h"

// BehaviorIntent：
// 行为意图数据包，只描述“这一帧想做什么”，不描述“具体怎么做”。
struct BehaviorIntent
{
    double moveX;
    double moveY;

    bool wantJump;
    bool wantSprint;
    bool wantInteract;

    BehaviorIntent();
};

// PlayerController：
// 负责把 InputManager 中的原始输入翻译成 BehaviorIntent。
class PlayerController
{
public:
    BehaviorIntent makeIntent(InputManager& input, bool god);
};
