#pragma once

#include "AnimationClipManager.h"
#include "Background.h"
#include "CameraFollow.h"
#include "CollisionHandle.h"
#include "Config.h"
#include "Controller.h"
#include "Entity.h"
#include "Input.h"
#include "MovementHandle.h"
#include "Renderer.h"
#include "Resource.h"
#include "TileMap.h"
#include "UI.h"

// Level:
// 当前关卡/场景管理器。
// 它持有当前关卡中的地图、背景、实体、UI 和各类 Handle，
// 但不亲自实现复杂的移动、碰撞、动画细节，只负责初始化和调度顺序。
class Level
{
private:
    ResourceManager resources;
    AnimationClipManager animationClips;

    TileMap tileMap;
    BackgroundManager backgroundManager;

    // 当前关卡实体列表。使用 vector 方便后续动态生成、清理和扩容。
    vector<Entity> entitys;

    // 当前关卡 UI 管理器，维护 UIElement 的父子关系、位置和可见状态。
    UIManager uiManager;

    // Debug 面板及其三个信息区在 UIManager 中的下标。
    int debugPanelIndex;
    int debugEntitySectionIndex;
    int debugRenderSectionIndex;
    int debugCameraSectionIndex;

    RenderFrameStats renderFrameStats;
    Renderer renderer;

    int controlTargerIndex;

    PlayerController playerController;
    MovementHandle movementHandle;
    CollisionHandle collisionHandle;

    int worldWidth;
    int worldHeight;

    // 背景视差使用的独立相机位置，避免直接使用被边界钳制后的渲染相机。
    double parallaxCameraX;
    double parallaxCameraY;

    // 背景视差初始参考点，用于计算背景层相对位移。
    double parallaxOriginX;
    double parallaxOriginY;

    // 以下历史状态缓存必须与 entitys.size() 同步，用于检测状态变化和重叠事件首次触发。
    vector<vector<bool>> lastOverlap;
    vector<bool> lastCollisionState;
    vector<bool> lastGroundState;
    vector<bool> lastSprintState;
    vector<bool> lastInAirState;
    vector<bool> lastJumpingState;
    vector<bool> lastAliveState;

    // 功能：根据当前相机跟随目标生成 Debug 面板数据。
    DebugPanelData buildDebugPanelData();

    // 功能：加载当前关卡需要的图片资源，并根据资源创建动画片段表。
    void initResources();

    // 功能：加载地图资源并根据地图尺寸设置世界范围。
    void initMap();

    // 功能：根据已加载的背景图片资源创建当前关卡的背景对象。
    void initBackground();

    // 功能：初始化当前关卡使用的 Debug UI 面板。
    void initUI();

    // 功能：设置实体 sprite 缩放、动画速度和碰撞盒缩放。
    void initEntitySettings();

    // 功能：初始化用于检测状态变化的历史缓存。
    void initLastStates();

    // 功能：清理所有存活实体的本帧临时状态。
    void clearEntityFrameState();

    // 功能：为实体生成行为意图，并执行移动、碰撞和动画更新。
    void updateEntities(InputManager& input);

    // 功能：切换当前由玩家输入控制的实体下标。
    void setControlTarget(int newTargetIndex);

    // 功能：处理玩家输入控制目标切换。
    void handleControlInput(InputManager& input);

    // 功能：处理相机跟随目标切换输入。
    void handleCameraInput(InputManager& input);

    // 功能：处理 Debug UI 临时显隐控制输入。
    void handleUIInput(InputManager& input);

    // 功能：切换整个 Debug 面板的显示状态。
    void toggleDebugPanelVisible();

    // 功能：切换 Debug 实体信息区。
    void toggleDebugEntitySectionVisible();

    // 功能：切换 Debug 渲染信息区。
    void toggleDebugRenderSectionVisible();

    // 功能：切换 Debug 相机信息区。
    void toggleDebugCameraSectionVisible();

    // 功能：切换指定 UI 元素自身的显示状态。
    void toggleUIElementVisible(int elementIndex);

    // 功能：处理渲染器调试显示开关输入。
    void handleRendererInput(InputManager& input);

    // 功能：根据输入状态更新相机跟随。
    void updateCamera(InputManager& input);

    // 功能：按固定参考视口宽度计算背景视差使用的横向相机中心。
    double getParallaxCameraCenterX();

    // 功能：按固定参考视口高度计算背景视差使用的纵向相机中心。
    double getParallaxCameraCenterY();

    // 功能：更新背景视差专用相机位置。
    void updateParallaxCamera();

    // 功能：检测实体状态变化并输出调试信息。
    void updateDebugStates();

    // 功能：检测实体重叠事件并向实体填充重叠列表。
    void updateOverlapEvents();

    // 功能：让实体各自独立处理重叠事件的具体玩法反馈。
    void resolveEntityOverlaps();

public:
    // 功能：初始化关卡实体列表和默认世界尺寸。
    Level();

    // 功能：在资源加载完成后，为所有实体同步初始化动画帧。
    void initEntityAnimations();

    // 功能：初始化关卡地图、背景、UI、实体设置和历史状态缓存。
    void init();

    // 功能：按固定顺序更新关卡中的输入、实体、相机、事件和 UI。
    void update(InputManager& input);

    // 功能：委托 Renderer 绘制当前关卡画面。
    void draw();
};
