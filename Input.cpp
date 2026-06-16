#include "Input.h"

// 功能：初始化键盘和鼠标输入缓存。
InputManager::InputManager()
{
    for (int i = 0; i < 256; i++)
    {
        keyNow[i] = false;
        keyLast[i] = false;
    }

    mouseLeftNow = false;
    mouseLeftLast = false;

    mouseX = WINDOW_WIDTH / 2;
    mouseY = WINDOW_HEIGHT / 2;
}

// 功能：刷新键盘、鼠标当前帧和上一帧输入状态。
void InputManager::update()
{
    for (int i = 0; i < 256; i++)
    {
        keyLast[i] = keyNow[i];
        keyNow[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
    }

    mouseLeftLast = mouseLeftNow;

    ExMessage msg;

    while (peekmessage(&msg, EX_MOUSE))
    {
        if (msg.message == WM_MOUSEMOVE)
        {
            mouseX = msg.x;
            mouseY = msg.y;
        }

        if (msg.message == WM_LBUTTONDOWN)
        {
            mouseLeftNow = true;
        }

        if (msg.message == WM_LBUTTONUP)
        {
            mouseLeftNow = false;
        }
    }
}

// 功能：判断指定按键当前是否处于按下状态。
bool InputManager::isKeyDown(int key)
{
    return keyNow[key];
}

// 功能：判断指定按键是否在当前帧刚刚按下。
bool InputManager::isKeyPressed(int key)
{
    return keyNow[key] && !keyLast[key];
}

// 功能：判断指定按键是否在当前帧刚刚松开。
bool InputManager::isKeyReleased(int key)
{
    return !keyNow[key] && keyLast[key];
}

// 功能：判断鼠标左键当前是否处于按下状态。
bool InputManager::isMouseLeftDown()
{
    return mouseLeftNow;
}

// 功能：判断鼠标左键是否在当前帧刚刚按下。
bool InputManager::isMouseLeftPressed()
{
    return mouseLeftNow && !mouseLeftLast;
}

// 功能：判断鼠标左键是否在当前帧刚刚松开。
bool InputManager::isMouseLeftReleased()
{
    return !mouseLeftNow && mouseLeftLast;
}

// 功能：获取鼠标当前屏幕 X 坐标。
int InputManager::getMouseX()
{
    return mouseX;
}

// 功能：获取鼠标当前屏幕 Y 坐标。
int InputManager::getMouseY()
{
    return mouseY;
}

// 功能：获取鼠标相对窗口中心的 X 偏移。
int InputManager::getMouseOffsetX()
{
    return mouseX - WINDOW_WIDTH / 2;
}

// 功能：获取鼠标相对窗口中心的 Y 偏移。
int InputManager::getMouseOffsetY()
{
    return mouseY - WINDOW_HEIGHT / 2;
}
