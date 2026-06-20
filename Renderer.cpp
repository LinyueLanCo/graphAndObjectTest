#include "Renderer.h"

#include "Camera.h"

RenderFrameStats::RenderFrameStats()
{
    backgroundSpriteCount = 0;
    tileSpriteCount = 0;
    entitySpriteCount = 0;
    totalSpriteCount = 0;
}

void RenderFrameStats::refreshTotal()
{
    totalSpriteCount =
        backgroundSpriteCount +
        tileSpriteCount +
        entitySpriteCount;
}

DebugPanelData::DebugPanelData()
{
    targetId = INVALID_ENTITY_ID;
    targetName = "";

    entityX = 0;
    entityY = 0;

    entityScreenX = 0;
    entityScreenY = 0;

    renderedBackgroundSprites = 0;
    renderedTileSprites = 0;
    renderedEntitySprites = 0;
    renderedTotalSprites = 0;

    cameraCenterX = 0;
    cameraCenterY = 0;
    cameraZoom = 1.0;

    viewLeft = 0;
    viewRight = 0;
    viewBottom = 0;
    viewTop = 0;
}

void Renderer::drawImageTileAlpha(
    int destX,
    int destY,
    int destW,
    int destH,
    IMAGE* imageSource,
    int srcX,
    int srcY,
    int srcW,
    int srcH
)
{
    if (imageSource == NULL)
    {
        return;
    }

    if (destW <= 0 || destH <= 0 || srcW <= 0 || srcH <= 0)
    {
        return;
    }

    BLENDFUNCTION blend;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    AlphaBlend(
        GetImageHDC(NULL),
        destX,
        destY,
        destW,
        destH,
        GetImageHDC(imageSource),
        srcX,
        srcY,
        srcW,
        srcH,
        blend
    );
}

void Renderer::drawEntityCollisionBox(Entity& entity)
{
    RectBox box = entity.getWorldCollisionBox();

    if (entity.hasCollisionState())
    {
        setlinecolor(RED);
    }
    else
    {
        setlinecolor(GREEN);
    }

    int screenLeft = gCamera.worldToScreenX(box.left);
    int screenRight = gCamera.worldToScreenX(box.right);

    int screenTop = gCamera.worldToScreenY(box.top);
    int screenBottom = gCamera.worldToScreenY(box.bottom);

    rectangle(screenLeft, screenTop, screenRight, screenBottom);
}

void Renderer::drawRenderBounds(int x, int y, int w, int h, COLORREF color)
{
    if (!showRenderBounds)
    {
        return;
    }

    if (w <= 0 || h <= 0)
    {
        return;
    }

    setlinecolor(color);
    rectangle(x, y, x + w, y + h);
}

Renderer::Renderer()
{
    showCollisionBox = true;
    showTileCollisionBox = false;
    showRenderBounds = false;
}

void Renderer::setShowCollisionBox(bool value)
{
    showCollisionBox = value;
}

void Renderer::setShowTileCollisionBox(bool value)
{
    showTileCollisionBox = value;
}

void Renderer::toggleCollisionBox()
{
    showCollisionBox = !showCollisionBox;
}

void Renderer::toggleTileCollisionBox()
{
    showTileCollisionBox = !showTileCollisionBox;
}

void Renderer::toggleRenderBounds()
{
    showRenderBounds = !showRenderBounds;
}

bool Renderer::getShowCollisionBox() const
{
    return showCollisionBox;
}

bool Renderer::getShowTileCollisionBox() const
{
    return showTileCollisionBox;
}

bool Renderer::drawSprite(const sprite& targetSprite, COLORREF renderBoundsColor)
{
    if (!targetSprite.visible)
    {
        return false;
    }

    if (targetSprite.imageSource == NULL)
    {
        return false;
    }

    if (targetSprite.srcW <= 0 || targetSprite.srcH <= 0)
    {
        return false;
    }

    if (targetSprite.worldDrawW <= 0 || targetSprite.worldDrawH <= 0)
    {
        return false;
    }

    // 用 sprite 世界中心点和世界绘制尺寸，计算世界绘制矩形。
    double worldLeft = targetSprite.worldCenterX - targetSprite.worldDrawW / 2.0;
    double worldRight = targetSprite.worldCenterX + targetSprite.worldDrawW / 2.0;
    double worldTop = targetSprite.worldCenterY + targetSprite.worldDrawH / 2.0;
    double worldBottom = targetSprite.worldCenterY - targetSprite.worldDrawH / 2.0;

    // 视口裁剪判定 (Culling)：
    // 采用 AABB 矩形相交算法，对比 Sprite 的世界包围盒与当前相机的世界逻辑视口。
    // 只有在 X 轴和 Y 轴上同时有重叠，才说明该 Sprite 的至少一部分在屏幕可视区域内。
    // 这样判定能完美防止 Sprite 边缘在进出屏幕时产生“凭空消失”或“黑边穿帮”现象。
    bool isVisible = (worldRight >= gCamera.getViewLeft())   && // 精灵右边界超过视口左边界
                     (worldLeft <= gCamera.getViewRight())  && // 精灵左边界未超视口右边界
                     (worldTop >= gCamera.getViewBottom())  && // 精灵上边界超过视口下边界
                     (worldBottom <= gCamera.getViewTop());    // 精灵下边界未超视口上边界

    if (!isVisible)
    {
        return false; // 在屏幕外，直接丢弃，不执行任何屏幕坐标换算和渲染操作
    }

    int drawX = gCamera.worldToScreenX(worldLeft);
    int drawY = gCamera.worldToScreenY(worldTop);

    int drawRight = gCamera.worldToScreenX(worldRight);
    int drawBottom = gCamera.worldToScreenY(worldBottom);

    int screenDrawW = drawRight - drawX;
    int screenDrawH = drawBottom - drawY;

    if (screenDrawW < 1)
    {
        screenDrawW = 1;
    }

    if (screenDrawH < 1)
    {
        screenDrawH = 1;
    }

    // 根据源图裁剪矩形和屏幕目标矩形完成 Alpha 混合绘制。
    drawImageTileAlpha(
        drawX,
        drawY,
        screenDrawW,
        screenDrawH,
        targetSprite.imageSource->getImage(),
        targetSprite.srcX,
        targetSprite.srcY,
        targetSprite.srcW,
        targetSprite.srcH
    );

    // sprite 绘制边界来自 sprite 自身的世界绘制数据。
    drawRenderBounds(
        drawX,
        drawY,
        screenDrawW,
        screenDrawH,
        renderBoundsColor
    );

    return true;
}

// 批量绘制所有活跃实体的调试碰撞框
void Renderer::drawEntityCollisionBoxes(vector<Entity>& entities, const vector<size_t>& activeIndices)
{
    for (size_t idx : activeIndices)
    {
        if (!entities[idx].getIsAlive())
        {
            continue;
        }

        drawEntityCollisionBox(entities[idx]);
    }
}

void Renderer::drawUIElementPanel(const UIElement& element)
{
    if (!element.isVisible())
    {
        return;
    }

    drawUIBox(
        element.getBox(),
        RGB(255, 255, 255),
        RGB(180, 180, 180)
    );
}

void Renderer::drawDebugEntitySectionText(const UIElement& content, DebugPanelData data)
{
    if (!content.isVisible())
    {
        return;
    }

    UIBox box = content.getBox();

    setbkmode(TRANSPARENT);
    settextcolor(RGB(40, 40, 40));
    settextstyle(18, 0, _T("Mojangles"));

    int x = box.x;
    int y = box.y;
    int lineH = 22;

    TCHAR text[128];

    _stprintf_s(text, _T("Debug Target"));
    outtextxy(x, y, text);
    y += lineH;

    _stprintf_s(text, _T("Entity Name: %hs"), data.targetName.c_str());
    outtextxy(x, y, text);
    y += lineH;

    _stprintf_s(text, _T("Entity IID: %d"), data.targetId);
    outtextxy(x, y, text);
    y += lineH;

    _stprintf_s(text, _T("World Pos: %.1f, %.1f"), data.entityX, data.entityY);
    outtextxy(x, y, text);
    y += lineH;

    _stprintf_s(text, _T("Screen Pos: %d, %d"), data.entityScreenX, data.entityScreenY);
    outtextxy(x, y, text);
}

void Renderer::drawDebugRenderSectionText(const UIElement& content, DebugPanelData data)
{
    if (!content.isVisible())
    {
        return;
    }

    UIBox box = content.getBox();

    setbkmode(TRANSPARENT);
    settextcolor(RGB(40, 40, 40));
    settextstyle(18, 0, _T("Mojangles"));

    int x = box.x;
    int y = box.y;
    int lineH = 22;

    TCHAR text[128];

    _stprintf_s(text, _T("Render"));
    outtextxy(x, y, text);
    y += lineH;

    _stprintf_s(text, _T("Bg Sprites: %d"), data.renderedBackgroundSprites);
    outtextxy(x, y, text);
    y += lineH;

    _stprintf_s(text, _T("Tile Sprites: %d"), data.renderedTileSprites);
    outtextxy(x, y, text);
    y += lineH;

    _stprintf_s(text, _T("Entity Sprites: %d"), data.renderedEntitySprites);
    outtextxy(x, y, text);
    y += lineH;

    _stprintf_s(text, _T("Total Sprites: %d"), data.renderedTotalSprites);
    outtextxy(x, y, text);
}

void Renderer::drawDebugCameraSectionText(const UIElement& content, DebugPanelData data)
{
    if (!content.isVisible())
    {
        return;
    }

    UIBox box = content.getBox();

    setbkmode(TRANSPARENT);
    settextcolor(RGB(40, 40, 40));
    settextstyle(18, 0, _T("Mojangles"));

    int x = box.x;
    int y = box.y;
    int lineH = 22;

    TCHAR text[128];

    _stprintf_s(text, _T("Camera"));
    outtextxy(x, y, text);
    y += lineH;

    _stprintf_s(text, _T("Center: %.1f, %.1f"), data.cameraCenterX, data.cameraCenterY);
    outtextxy(x, y, text);
    y += lineH;

    _stprintf_s(text, _T("Zoom: %.2f"), data.cameraZoom);
    outtextxy(x, y, text);
    y += lineH;

    _stprintf_s(text, _T("View L/R: %.1f / %.1f"), data.viewLeft, data.viewRight);
    outtextxy(x, y, text);
    y += lineH;

    _stprintf_s(text, _T("View B/T: %.1f / %.1f"), data.viewBottom, data.viewTop);
    outtextxy(x, y, text);
}
