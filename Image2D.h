#pragma once
#include "Config.h"

// Image2D：
// 包装一张 EasyX IMAGE，并记录图片基础信息。
// 它只负责加载和提供图片资源，不负责绘制、不负责动画、不负责坐标。
class Image2D
{
private:
    IMAGE image;

    int width;
    int height;

public:
    // 功能：初始化一个空图片资源。
    Image2D();

    // 功能：从文件加载图片，并记录图片宽高。
    bool load(const TCHAR* path);

    // 功能：从文件加载图片，并按指定大小缩放到内存图片。
    bool load(const TCHAR* path, int loadW, int loadH);

    // 功能：获取 EasyX 原始图片指针，供底层绘制函数使用。
    IMAGE* getImage();

    // 功能：获取图片宽度。
    int getWidth() const;

    // 功能：获取图片高度。
    int getHeight() const;
};
