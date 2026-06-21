#pragma once

class Level;
class InputManager;

// 第一关（Level 1）专属关卡事件函数声明
void level1_InitEvent(Level& level);
void level1_UpdateEvent(Level& level, InputManager& input);
