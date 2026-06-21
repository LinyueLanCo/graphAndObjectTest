#include "Renderer.h"

#include "Camera.h"
#include "DialogueBox.h"
#include "Image2D.h"

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

void Renderer::drawDebugSectionText(const UIElement& content, const std::vector<std::string>& lines)
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

    for (size_t i = 0; i < lines.size(); ++i)
    {
        TCHAR text[128];
        _stprintf_s(text, _T("%hs"), lines[i].c_str());
        outtextxy(x, y, text);
        y += lineH;
    }
}

void Renderer::drawTileCollisionBoxes(const TileMap& tileMap)
{
    for (const TileInstance& tile : tileMap.getTileInstances())
    {
        if (!tile.visible || tile.tileId == TILE_EMPTY || tile.collisionType == TILE_COLLISION_NONE)
        {
            continue;
        }

        if (tile.collisionType == TILE_COLLISION_FULL_SOLID)
        {
            setlinecolor(YELLOW);
        }
        else
        {
            setlinecolor(GREEN);
        }

        RectBox box = tileMap.getTileInstanceCollisionWorldBox(tile);

        int screenLeft = gCamera.worldToScreenX(box.left);
        int screenRight = gCamera.worldToScreenX(box.right);
        int screenTop = gCamera.worldToScreenY(box.top);
        int screenBottom = gCamera.worldToScreenY(box.bottom);

        rectangle(screenLeft, screenTop, screenRight, screenBottom);
    }
}

void Renderer::drawDialogueBox(const DialogueBox& dialogueBox)
{
    if (!dialogueBox.isVisible())
    {
        return;
    }

    const DialogueConfig& config = dialogueBox.getConfig();
    Image2D* fontTexture = dialogueBox.getFontTexture();
    const std::string& displayText = dialogueBox.getDisplayText();

    // 1. 绘制复古双边框深色底座圆角面板背景
    UIBox box = dialogueBox.getBox();
    drawUIBox(box, config.bgColor, config.borderColor);

    // 绘制内嵌的第二层圆角内边框线框 (无填充)
    int indent = 4;
    setlinecolor(config.borderColor);
    roundrect(
        box.x + indent,
        box.y + indent,
        box.x + box.w - indent,
        box.y + box.h - indent,
        22,
        22
    );

    if (fontTexture == nullptr || displayText.empty())
    {
        return;
    }

    // 2. 逐字换行排版并直接调用公开化的底层贴图切片绘制
    int drawX = box.x + config.paddingLeft;
    int drawY = box.y + config.paddingTop;
    
    int drawCharW = (int)(config.charWidth * config.charScale);
    int drawCharH = (int)(config.charHeight * config.charScale);

    // 最大可绘制宽度限制，超过此坐标即自动换行
    int maxRight = box.x + box.w - config.paddingLeft;

    static const std::string FONT_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ    0123456789.,:?!()+- ";

    for (char c : displayText)
    {
        if (c == '\n')
        {
            // 显式换行
            drawX = box.x + config.paddingLeft;
            drawY += drawCharH + config.lineSpacing;
            continue;
        }

        if (c == ' ')
        {
            // 空格不绘制图像，直接横坐标步进
            drawX += drawCharW + config.charSpacing;
            
            // 自动折行检测
            if (drawX + drawCharW > maxRight)
            {
                drawX = box.x + config.paddingLeft;
                drawY += drawCharH + config.lineSpacing;
            }
            continue;
        }

        // 查找字符在映射对照串中的索引
        size_t found = FONT_CHARS.find(c);
        if (found != std::string::npos)
        {
            int index = (int)found;
            int col = index % 10;
            int row = index / 10;
            int srcX = col * config.charWidth;
            int srcY = row * config.charHeight;

            // 统一调用已经公有化的 drawImageTileAlpha 完成 Alpha 混合透明像素绘制
            this->drawImageTileAlpha(
                drawX,
                drawY,
                drawCharW,
                drawCharH,
                fontTexture->getImage(),
                srcX,
                srcY,
                config.charWidth,
                config.charHeight
            );
        }

        // 步进到下一个字位置
        drawX += drawCharW + config.charSpacing;

        // 自动换行检查：如果下一个字的位置超出了对话框右边缘，则提前换行
        if (drawX + drawCharW > maxRight)
        {
            drawX = box.x + config.paddingLeft;
            drawY += drawCharH + config.lineSpacing;
        }
    }
}
