#pragma once

#include <vector>
#include "Sprite.h"

// SpriteType：定义 Sprite 属于哪种游戏系统，用于帧统计数据归类
enum SpriteType
{
    SPRITE_TYPE_BACKGROUND,
    SPRITE_TYPE_TILE,
    SPRITE_TYPE_ENTITY
};

// RenderItem：包含渲染所需的精灵数据和辅助渲染信息
struct RenderItem
{
    sprite spriteData;
    COLORREF debugBoundsColor;
    SpriteType type;
};

// 前置声明 RenderFrameStats 与 Renderer
struct RenderFrameStats;
class Renderer;

// RenderQueue：渲染队列容器，负责收集、稳定排序以及统一分发绘制
class RenderQueue
{
private:
    std::vector<RenderItem> items;

public:
    RenderQueue();
    ~RenderQueue();

    // 清空队列所有元素
    void clear();

    // 提交一个 Sprite 拷贝
    void submit(const sprite& sp, SpriteType type, COLORREF debugColor = RGB(0, 220, 255));

    // 根据 Sprite 的 zIndex 进行稳定排序
    void sort();

    // 统一执行绘制，并更新当前帧的统计数据
    int drawAll(Renderer& renderer, RenderFrameStats& stats);

    // 获取当前队列中的元素数量
    size_t size() const;
};
