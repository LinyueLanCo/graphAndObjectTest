#pragma once

#include "AnimationClipManager.h"
#include "Background.h"
#include "CameraFollow.h"
#include "CollisionHandle.h"
#include "Config.h"
#include "Controller.h"
#include "EntityManager.h"
#include "Input.h"
#include "MovementHandle.h"
#include "Renderer.h"
#include "Resource.h"
#include "TileMap.h"
#include "UI.h"

// Level: 关卡大舞台（场景控制器）。
// 它是当前关卡的核心组织者，整合了地图、视差背景、实体、UI以及物理/渲染引擎。
// 它就像导演一样，在每一帧的 update 中按照固定流程调用各个模块，指挥大家协作。
class Level
{
private:
    ResourceManager resources;               // 资源箱：用来加载和保存当前关卡需要的所有图片和文本路径
    AnimationClipManager animationClips;     // 动画本子：根据图片资源切割成的角色动作帧片段表

    TileMap tileMap;                         // 瓦片地图：管理格子地图数据（墙壁、单向平台、空气等物理阻挡）
    BackgroundManager backgroundManager;     // 视差背景管家：让天空、云、树木层以不同的速度位移，形成 3D 深度感

    EntityManager entityManager;             // 演员池：管理关卡中所有的实体角色（玩家、金币、旗杆）

    UIManager uiManager;                     // UI大本营：管理整个关卡的调试 UI 元素、父子层级关系和渐变动画

    // 各种 UI 块在 UIManager 里的下标，用来开关和做动画
    int debugPanelIndex;                     // 调试面板大底框的下标
    int debugEntitySectionIndex;             // 演员数据面板的下标
    int debugRenderSectionIndex;             // 渲染数据面板的下标
    int debugCameraSectionIndex;             // 相机数据面板的下标

    RenderFrameStats renderFrameStats;       // 渲染计数器：统计本帧画了多少背景、有多少实体、多少瓦片
    Renderer renderer;                       // 画笔：负责调用 EasyX 将各类游戏精灵、文本及 UI 框真正绘制到屏幕上

    EntityID controlledPlayerId;             // 当前操控的演员 EntityID
    
    // 重叠日志历史缓存：记录上一帧已经发生重叠的配对 ID（如 "Player1_Coin1"），用 vector 记录，防止每帧重复打印刷屏
    std::vector<std::string> lastOverlapPairs;

    PlayerController playerController;       // 控制器翻译官：把键盘的 WASD/空格等输入翻译为玩家想移动跳跃的意图
    MovementHandle movementHandle;           // 物理发动机：根据意图更新实体的坐标和重力
    CollisionHandle collisionHandle;         // 碰撞检测姬：计算实体会不会撞墙、能够移动多远

    int worldWidth;                          // 关卡世界的像素总宽度
    int worldHeight;                         // 关卡世界的像素总高度


    // 辅助功能：根据当前相机跟随的演员，组装当前帧的调试数据（坐标、像素、相机缩放等）
    DebugPanelData buildDebugPanelData();

    // 辅助功能：将本关卡要用的素材图片物理路径登记到 Resource 箱子中
    void initResources();

    // 辅助功能：读取格子地图 map.txt，并根据地图尺寸设定关卡世界的实际宽度和高度
    void initMap();

    // 辅助功能：添加多层视差背景，并设置好各自的滚动倍率（天空滚得最慢，花草滚得快）
    void initBackground();

    // 辅助功能：拼装屏幕右上角的整个 Debug 控制台 UI 架构
    void initUI();

    // 步骤函数：遍历活跃名单，清理所有存活实体的本帧物理接触标志（准备迎接新的一帧物理计算）
    void clearEntityFrameState();

    // 步骤函数：核心更新——根据键盘输入，移动所有活跃演员，做碰撞修正，切换动画状态
    // input: 输入管理箱，用来查询本帧哪些键被按下了
    void updateEntities(InputManager& input);

    void setControlTarget(EntityID targetId);
    void setControlTargetByName(const std::string& name);
    void setCameraFollowTargetByName(const std::string& name);

    // 步骤函数：监控按键 1、2、3、4，允许玩家在游戏中动态切人控制
    void handleControlInput(InputManager& input);

    // 步骤函数：监控按键 F1 - F4，允许镜头聚焦到不同的演员身上
    void handleCameraInput(InputManager& input);

    // 步骤函数：监控按键 F8 - F11，触发调试 UI 各个分块的弹出和缩回渐变动画
    void handleUIInput(InputManager& input);

    // 一系列辅助开关 UI 的缓动触发函数
    void toggleDebugPanelVisible();
    void toggleDebugEntitySectionVisible();
    void toggleDebugRenderSectionVisible();
    void toggleDebugCameraSectionVisible();
    void toggleUIElementVisible(int elementIndex); // 显隐指定的 UI 块

    // 步骤函数：监控 F5 - F7，开关碰撞盒、地图瓦片边缘、渲染边界的绘制状态
    void handleRendererInput(InputManager& input);

    // 步骤函数：更新相机的坐标（缓动跟随当前目标，并把它钳制在世界边界里）
    void updateCamera(InputManager& input);


    // 步骤函数：对比当前帧与上一帧的物理标记（如是否触碰、是否落地、是否起跳、死亡等），发生改变时在窗口打印日志
    void updateDebugStates();

    // 步骤函数：双重循环检测实体两两之间是否有 AABB 重叠，填充 entities 内部的重叠信息，避免重复打印日志
    void updateOverlapEvents();

    // 步骤函数：让发生碰撞的实体自己去读取自己的重叠列表，决定是否加分、自毁或升旗
    void resolveEntityOverlaps();


public:
    // 构造函数：初始化场景的基本状态默认值
    Level();

    // 步骤函数：让所有的活跃实体绑定他们动画包里的第一段待机动画
    void initEntityAnimations();

    // 关卡总装配：被外部 game.cpp 启动时调用，按顺序完全加载资源、地图、UI 和演员
    void init();

    // 每帧的核心驱动逻辑：按严格顺序调度键盘、移动、镜头、碰撞交互、UI 渐变
    // input: 外部传入的输入管理箱
    void update(InputManager& input);

    // 每帧的画面渲染：依次画背景、瓦片地图、活跃演员角色、UI 调试层
    void draw();
};
