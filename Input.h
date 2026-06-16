#pragma once

#include "Config.h"

// InputManager：
// 统一采集键盘和鼠标输入，并提供当前帧、刚按下、刚松开的查询接口。
class InputManager
{
private:
    bool keyNow[256];
    bool keyLast[256];

    bool mouseLeftNow;
    bool mouseLeftLast;

    int mouseX;
    int mouseY;

public:
    InputManager();

    void update();

    bool isKeyDown(int key);
    bool isKeyPressed(int key);
    bool isKeyReleased(int key);

    bool isMouseLeftDown();
    bool isMouseLeftPressed();
    bool isMouseLeftReleased();

    int getMouseX();
    int getMouseY();
    int getMouseOffsetX();
    int getMouseOffsetY();
};
