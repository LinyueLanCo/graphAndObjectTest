#include "Controller.h"

// 功能：初始化一帧空的行为意图。
BehaviorIntent::BehaviorIntent()
{
    moveX = 0;
    moveY = 0;

    wantJump = false;
    wantSprint = false;
    wantInteract = false;
}

// 功能：把玩家输入转换为本帧行为意图。
BehaviorIntent PlayerController::makeIntent(InputManager& input, bool god)
{
    BehaviorIntent intent;

    if (input.isKeyDown(VK_LEFT))
    {
        intent.moveX = -1;
    }

    if (input.isKeyDown(VK_RIGHT))
    {
        intent.moveX = 1;
    }

    if (god)
    {
        if (input.isKeyDown(VK_UP))
        {
            intent.moveY = 1;
        }

        if (input.isKeyDown(VK_DOWN))
        {
            intent.moveY = -1;
        }
    }

    intent.wantSprint = input.isKeyDown(VK_SHIFT);
    intent.wantJump = input.isKeyPressed(VK_SPACE);
    intent.wantInteract = input.isKeyPressed('E');

    return intent;
}
