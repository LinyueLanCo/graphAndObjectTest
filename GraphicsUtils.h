#pragma once
#include "Config.h"

// 功能：注册项目内置字体，供 EasyX 文本绘制使用。
void loadUIFont();

// 功能：按原图尺寸绘制带 Alpha 通道的图片。
void putimage_alpha(int x, int y, IMAGE* img);

// 功能：按指定尺寸缩放绘制带 Alpha 通道的图片。
void putimage_alpha(int x, int y, int drawW, int drawH, IMAGE* img);