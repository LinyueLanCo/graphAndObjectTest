#pragma once

#include "Background.h"
#include "Config.h"
#include "Entity.h"
#include "TileMap.h"
#include "UI.h"

// RenderFrameStats：
// 记录当前帧真实通过 Renderer::drawSprite 绘制成功的 sprite 数量。
struct RenderFrameStats
{
    int backgroundSpriteCount;
    int tileSpriteCount;
    int entitySpriteCount;
    int totalSpriteCount;

    // 功能：初始化当前帧渲染统计数据。
    RenderFrameStats();

    // 功能：根据各类型 sprite 数量重新计算总绘制数量。
    void refreshTotal();
};

// DebugPanelData：
// Debug 面板一帧要显示的数据快照，数据由 Level 在当前帧收集后交给 Renderer 显示。
struct DebugPanelData
{
    int targetIndex;

    double entityX;
    double entityY;

    int entityScreenX;
    int entityScreenY;

    int renderedBackgroundSprites;
    int renderedTileSprites;
    int renderedEntitySprites;
    int renderedTotalSprites;

    double cameraCenterX;
    double cameraCenterY;
    double cameraZoom;

    double viewLeft;
    double viewRight;
    double viewBottom;
    double viewTop;

    DebugPanelData();
};

// Renderer：
// 统一管理当前关卡中的可渲染对象。
// 它负责把 sprite / tile / UI 等数据绘制到屏幕，并集中处理实体调试碰撞框绘制。
// 实体自身不再直接调用 EasyX 绘图函数。
class Renderer
{
private:
    // 是否显示实体逻辑碰撞盒。
    bool showCollisionBox;

    // 是否显示 tile 逻辑碰撞盒。
    bool showTileCollisionBox;

    // 是否显示所有可绘制对象的屏幕绘制边界。
    bool showRenderBounds;

    // 功能：从图集中裁剪指定区域，并以 Alpha 混合绘制到屏幕目标矩形。
    void drawImageTileAlpha(
        int destX,
        int destY,
        int destW,
        int destH,
        IMAGE* imageSource,
        int srcX,
        int srcY,
        int srcW,
        int srcH
    );

    // 功能：绘制实体当前世界碰撞盒的调试矩形。
    void drawEntityCollisionBox(Entity& entity);

    // 功能：在屏幕坐标中绘制渲染对象的实际绘制边界。
    void drawRenderBounds(int x, int y, int w, int h, COLORREF color);

public:
    // 功能：初始化渲染器的调试绘制开关。
    Renderer();

    // 功能：设置是否绘制实体碰撞框。
    void setShowCollisionBox(bool value);

    // 功能：设置是否绘制 tile 碰撞框。
    void setShowTileCollisionBox(bool value);

    // 功能：切换实体碰撞框显示状态。
    void toggleCollisionBox();

    // 功能：切换 tile 碰撞框显示状态。
    void toggleTileCollisionBox();

    // 功能：切换可绘制对象的绘制边界框显示状态。
    void toggleRenderBounds();

    // 功能：绘制 BackgroundManager 中本帧已经展开好的背景对象，并返回真实绘制成功的 background sprite 数量。
    int drawBackgroundObjects(BackgroundManager& backgroundManager);

    // 功能：根据 TileInstance 生成通用 sprite，并交给统一 sprite 绘制接口。
    bool drawTileInstance(TileMap& tileMap, const TileInstance& tile);

    // 功能：逐个绘制当前地图中的 tile 实例，并返回真实绘制成功的 tile sprite 数量。
    int drawTileMap(TileMap& tileMap);

    // 功能：根据 sprite 自身保存的世界绘制数据绘制单帧图像。
    bool drawSprite(const sprite& targetSprite, COLORREF renderBoundsColor = RGB(0, 220, 255));

    // 功能：绘制实体列表中的所有存活实体，并返回真实绘制成功的 entity sprite 数量。
    int drawEntities(vector<Entity>& entitys);

    // 功能：绘制一个通用 UIElement 面板。
    void drawUIElementPanel(const UIElement& element);

    // 功能：绘制 Debug 面板中的实体数据区。
    void drawDebugEntitySectionText(const UIElement& content, DebugPanelData data);

    // 功能：绘制 Debug 面板中的渲染数据区。
    void drawDebugRenderSectionText(const UIElement& content, DebugPanelData data);

    // 功能：绘制 Debug 面板中的相机数据区。
    void drawDebugCameraSectionText(const UIElement& content, DebugPanelData data);
};
