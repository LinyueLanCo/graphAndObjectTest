#include "RenderQueue.h"
#include "Renderer.h"
#include <algorithm>

RenderQueue::RenderQueue()
{
}

RenderQueue::~RenderQueue()
{
}

void RenderQueue::clear()
{
    items.clear();
}

void RenderQueue::submit(const sprite& sp, SpriteType type, COLORREF debugColor)
{
    items.push_back({ sp, debugColor, type });
}

void RenderQueue::sort()
{
    // 稳定排序：在 zIndex 相同时保留其原有插入顺序
    std::stable_sort(items.begin(), items.end(), [](const RenderItem& a, const RenderItem& b) {
        return a.spriteData.zIndex < b.spriteData.zIndex;
    });
}

int RenderQueue::drawAll(Renderer& renderer, RenderFrameStats& stats)
{
    stats.backgroundSpriteCount = 0;
    stats.tileSpriteCount = 0;
    stats.entitySpriteCount = 0;
    stats.totalSpriteCount = 0;

    int totalDrawCount = 0;
    for (const auto& item : items)
    {
        if (renderer.drawSprite(item.spriteData, item.debugBoundsColor))
        {
            totalDrawCount++;
            if (item.type == SPRITE_TYPE_BACKGROUND)
            {
                stats.backgroundSpriteCount++;
            }
            else if (item.type == SPRITE_TYPE_TILE)
            {
                stats.tileSpriteCount++;
            }
            else if (item.type == SPRITE_TYPE_ENTITY)
            {
                stats.entitySpriteCount++;
            }
        }
    }
    stats.refreshTotal();
    return totalDrawCount;
}

size_t RenderQueue::size() const
{
    return items.size();
}
