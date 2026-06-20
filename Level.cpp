#include "Level.h"

#include "Camera.h"

// contains: 一个小帮手函数，用来判断某个字符串是不是在 vector 里。
// 主要是为了配合调试日志去重，传入历史名单 vec 和碰撞 ID，如果找到就返回 true。
static bool contains(const std::vector<std::string>& vec, const std::string& key)
{
    for (const auto& item : vec)
    {
        if (item == key)
        {
            return true; // 找到了！说明之前就已经碰上了
        }
    }
    return false; // 没找到，说明是个新来的碰撞
}

// 收集当前帧的游戏状态（如焦点角色的像素位置、渲染消耗、相机视口范围等），用作 Debug UI 的数据显示
DebugPanelData Level::buildDebugPanelData()
{
    DebugPanelData data;

    // 拿到焦点跟随目标的 ID
    data.targetId = gCameraFollowTargetId;

    // 如果这个目标演员还活着，把它的世界坐标和屏幕坐标取出来传给 UI 面板
    Entity* target = entityManager.getEntity(gCameraFollowTargetId);
    if (target)
    {
        data.targetName = target->getName();
        data.entityX = target->getX();
        data.entityY = target->getY();

        data.entityScreenX = gCamera.worldToScreenX(target->getX());
        data.entityScreenY = gCamera.worldToScreenY(target->getY());
    }

    // 统计本帧画图开销数据
    data.renderedBackgroundSprites = renderFrameStats.backgroundSpriteCount;
    data.renderedTileSprites = renderFrameStats.tileSpriteCount;
    data.renderedEntitySprites = renderFrameStats.entitySpriteCount;
    data.renderedTotalSprites = renderFrameStats.totalSpriteCount;

    // 获取相机的中心点和当前焦距缩放倍率
    data.cameraCenterX = gCamera.centerX;
    data.cameraCenterY = gCamera.centerY;
    data.cameraZoom = gCamera.zoom;

    // 获取当前相机世界视口的四个边界线坐标
    data.viewLeft = gCamera.getViewLeft();
    data.viewRight = gCamera.getViewRight();
    data.viewBottom = gCamera.getViewBottom();
    data.viewTop = gCamera.getViewTop();

    return data;
}

// 关卡构造函数：初始化状态控制变量默认值
Level::Level()
{
    controlledPlayerId = INVALID_ENTITY_ID;

    // 初始化 UI 元素的索引。一开始还没创建，所以记为 -1
    debugPanelIndex = -1;
    debugEntitySectionIndex = -1;
    debugRenderSectionIndex = -1;
    debugCameraSectionIndex = -1;

    worldWidth = WINDOW_WIDTH;
    worldHeight = WINDOW_HEIGHT;
}

// 演员动画皮肤初始化：在关卡资源准备妥当后，给对象池中活着的实体同步绑定其动画包里的第一段待机动画
void Level::initEntityAnimations()
{
    auto& entities = entityManager.getEntities();
    for (size_t idx : entityManager.getActiveIndices())
    {
        entities[idx].initAnimationFromAnimator();
    }
}

// 关卡大初始化：加载本关所需的全部游戏资源
void Level::init()
{
    // 1. 登记图片资源路径并载入内存
    initResources();

    // 1.5. 加载实体模板
    entityManager.loadTemplates("assets/data/entity_templates.json");

    // 2. 从文本解析格子地图，设定世界总像素尺寸
    initMap();

    // 3. 从 JSON 配置文件加载本关的初始实体，放入对象池大箱子里
    entityManager.loadEntities("assets/data/entities.json", animationClips);

    // Find checkpoint and offset its collision box (rule-based lookup)
    for (size_t idx : entityManager.getActiveIndices())
    {
        if (entityManager.getEntities()[idx].getEntityType() == CHECKPOINT)
        {
            entityManager.getEntities()[idx].setCollisionBoxOffset(0.0, -30.0);
        }
    }

    // 4. 拼装屏幕右上角的调试 UI 控制台
    initUI();

    // 5. 每次进关卡时，记得把碰撞打印的历史名单清空，准备开始新的计算
    lastOverlapPairs.clear();

    // Find default controlled player
    for (size_t idx : entityManager.getActiveIndices())
    {
        if (entityManager.getEntities()[idx].isControlled())
        {
            controlledPlayerId = entityManager.getEntities()[idx].getId();
            break;
        }
    }
    if (controlledPlayerId == INVALID_ENTITY_ID)
    {
        for (size_t idx : entityManager.getActiveIndices())
        {
            if (entityManager.getEntities()[idx].getEntityType() == PLAYER)
            {
                controlledPlayerId = entityManager.getEntities()[idx].getId();
                break;
            }
        }
    }

    // 8. 默认激活被控角色的键盘操纵权限
    setControlTarget(controlledPlayerId);

    // 9. 摆放天空、云朵和树木图层
    initBackground();
}

// Level::update: 每一帧的核心更新流程（数据流动中枢）。
// 一帧运转的详细流程如下：
// 1. 清除所有实体上一帧的物理状态标记（是否在空中等）。
// 2. 捕获切人控制的键盘按键（1、2、3、4）。
// 3. 更新实体位移：计算意图，调用 MovementHandle 和 CollisionHandle 计算位置并应用动画。
// 4. 处理镜头切换和 UI 显隐按键（F1-F4, F8-F11 等）。
// 5. 缓缓移动镜头让其平滑跟上被控角色，并更新视差背景滚动。
// 6. 输出状态日志（如落地、跳跃、死亡等）。
// 7. 检测重叠事件（算算谁碰到了谁），让实体在内部标记发生了碰撞。
// 8. 处理串联事件：如果旗杆动画播放完毕，就通知 EntityManager 在队列里登记生成金币。
// 9. 调用 processSpawns() 安全完成垃圾回收和新生实体的复活。
// 10. 更新 UI 各面板的缓动动画位置。
void Level::update(InputManager& input)
{
    if (input.isMouseLeftPressed())
    {
        cout << "鼠标左键按下了，坐标在: "
            << input.getMouseX()
            << " "
            << input.getMouseY()
            << endl;
    }

    // 清理上帧的物理临时痕迹
    clearEntityFrameState();

    // 监测切人按键
    handleControlInput(input);

    // 物理移动与动画状态更新
    updateEntities(input);

    // 捕获镜头和 UI 调试按键的输入
    handleCameraInput(input);
    handleUIInput(input);
    handleRendererInput(input);

    // 更新镜头和平滑背景视差
    updateCamera(input);
    if (gCamera.dx != 0.0 || gCamera.dy != 0.0)
    {
        cout << "Camera Move: vx=" << gCamera.dx << ", vy=" << gCamera.dy 
             << ", center=" << gCamera.centerX << ", " << gCamera.centerY << endl;
    }
    backgroundManager.updateRuntimeTransforms(gCamera.dx, gCamera.dy);

    // 轮询并打印状态转移日志
    updateDebugStates();

    // 进行重叠交互判定与响应
    updateOverlapEvents();
    resolveEntityOverlaps();

    // 事件联动：检查是否有旗子刚刚播完升旗动画，如果是，立马让管家生成银币（Coin2）
    static int spawnedCoinCounter = 1;
    auto& entities = entityManager.getEntities();
    for (size_t idx : entityManager.getActiveIndices())
    {
        Entity& ent = entities[idx];
        if (ent.getEntityType() == CHECKPOINT && ent.flagActivatedJustNow)
        {
            ent.flagActivatedJustNow = false; // 消费掉这个标记，重置它

            entityManager.queueSpawnEntity(
                ent.getX(),
                ent.getY() + 64.0,     // 放置在新位置
                "Banana"
            );
        }
    }

    // 帧末垃圾回收与动态生成落地
    entityManager.processSpawns(animationClips);

    // UI 数据变化缓动更新
    uiManager.update();
}

void Level::draw()
{
    // 1. 清空上一帧的渲染队列
    renderQueue.clear();

    // 2. 收集各个系统的 Sprite 拷贝
    backgroundManager.collectSprites(renderQueue);
    tileMap.collectSprites(renderQueue);
    entityManager.collectSprites(renderQueue);

    // 3. 按照 zIndex 进行稳定排序
    renderQueue.sort();

    // 4. 统一分发给 Renderer 绘制所有 Sprite 并更新帧统计数据
    renderQueue.drawAll(renderer, renderFrameStats);

    // 5. 调试后期处理：绘制实体和地图的碰撞框覆盖层
    if (renderer.getShowCollisionBox())
    {
        renderer.drawEntityCollisionBoxes(entityManager.getEntities(), entityManager.getActiveIndices());
    }
    if (renderer.getShowTileCollisionBox())
    {
        tileMap.drawDebugCollisionBoxes();
    }

    // Debug 面板父级负责整体背景；子 section 的最终可见性由 UIManager 按父级链路判断。
    if (uiManager.isElementEffectivelyVisible(debugPanelIndex))
    {
        renderer.drawUIElementPanel(uiManager.getElement(debugPanelIndex));
    }

    DebugPanelData data = buildDebugPanelData();

    if (uiManager.isElementEffectivelyVisible(debugEntitySectionIndex))
    {
        renderer.drawDebugEntitySectionText(
            uiManager.getElement(debugEntitySectionIndex),
            data
        );
    }

    if (uiManager.isElementEffectivelyVisible(debugRenderSectionIndex))
    {
        renderer.drawDebugRenderSectionText(
            uiManager.getElement(debugRenderSectionIndex),
            data
        );
    }

    if (uiManager.isElementEffectivelyVisible(debugCameraSectionIndex))
    {
        renderer.drawDebugCameraSectionText(
            uiManager.getElement(debugCameraSectionIndex),
            data
        );
    }

}

void Level::initResources()
{
    resources.loadLevelResources();
    animationClips.init("assets/data/animations.json", resources);
}

void Level::initMap()
{
    tileMap.setTileSize(16, 16, 48, 48);
    tileMap.loadTileset(resources.getImage2D("tileset"));
    tileMap.loadFromText(resources.getTextContent("map_main"));

    worldWidth = tileMap.getworldWidth();
    worldHeight = tileMap.getWOrldHeight();

    if (worldWidth < WINDOW_WIDTH)
    {
        worldWidth = WINDOW_WIDTH;
    }

    if (worldHeight < WINDOW_HEIGHT)
    {
        worldHeight = WINDOW_HEIGHT;
    }
}

void Level::initBackground()
{
    backgroundManager.clear();

    Image2D* skyImage = resources.getImage2D("bg_sky");
    Image2D* cloudsImage = resources.getImage2D("bg_clouds");
    Image2D* flora1Image = resources.getImage2D("bg_flora1");
    Image2D* flora2Image = resources.getImage2D("bg_flora2");

    double backgroundCenterX = gCamera.centerX;
    double backgroundCenterY = gCamera.centerY;

    backgroundManager.addObjectFromImage2D(
        skyImage,
        0,
        0.0,
        0.0,
        false,
        BACKGROUND_REPEAT_X,
        backgroundCenterX,
        backgroundCenterY,
        skyImage != NULL ? skyImage->getWidth() : WINDOW_WIDTH,
        skyImage != NULL ? skyImage->getHeight() : WINDOW_HEIGHT
    );


    backgroundManager.addObjectFromImage2D(
        cloudsImage,
        1,
        0.15,
        0.15,
        true,
        BACKGROUND_REPEAT_X,
        backgroundCenterX,
        backgroundCenterY,
        cloudsImage != NULL ? cloudsImage->getWidth() : WINDOW_WIDTH,
        cloudsImage != NULL ? cloudsImage->getHeight() : WINDOW_HEIGHT,
        0.5
    );

    backgroundManager.addObjectFromImage2D(
        flora1Image,
        2,
        0.35,
        0.35,
        true,
        BACKGROUND_REPEAT_X,
        backgroundCenterX,
        backgroundCenterY,
        flora1Image != NULL ? flora1Image->getWidth() : WINDOW_WIDTH,
        flora1Image != NULL ? flora1Image->getHeight() : WINDOW_HEIGHT
    );

    backgroundManager.addObjectFromImage2D(
        flora2Image,
        3,
        0.65,
        0.65,
        true,
        BACKGROUND_REPEAT_X,
        backgroundCenterX,
        backgroundCenterY,
        flora2Image != NULL ? flora2Image->getWidth() : WINDOW_WIDTH,
        flora2Image != NULL ? flora2Image->getHeight() : WINDOW_HEIGHT
    );
}

void Level::initUI()
{
    // Debug 面板是顶层 UI，相对于窗口右上角定位。
    UIElement debugPanel;
    debugPanel.init(420, 520, UI_TOP_RIGHT, 24, 24);

    debugPanelIndex = uiManager.addElement(debugPanel);

    // Debug Entity 区域：显示当前相机跟随目标实体的数据。
    UIElement debugEntitySection;
    debugEntitySection.init(388, 110, UI_TOP_LEFT, 16, 16);
    debugEntitySection.setParentIndex(debugPanelIndex);
    debugEntitySection.refreshTargetByParentBox(uiManager.getElement(debugPanelIndex).getBox());
    debugEntitySection.snapToTarget();
    debugEntitySectionIndex = uiManager.addElement(debugEntitySection);

    // Debug Render 区域：显示当前渲染相关数据。
    UIElement debugRenderSection;
    debugRenderSection.init(388, 130, UI_TOP_LEFT, 16, 140);
    debugRenderSection.setParentIndex(debugPanelIndex);
    debugRenderSection.refreshTargetByParentBox(uiManager.getElement(debugPanelIndex).getBox());
    debugRenderSection.snapToTarget();
    debugRenderSectionIndex = uiManager.addElement(debugRenderSection);

    // Debug Camera 区域：显示当前相机和视口数据。
    UIElement debugCameraSection;
    debugCameraSection.init(388, 180, UI_TOP_LEFT, 16, 300);
    debugCameraSection.setParentIndex(debugPanelIndex);
    debugCameraSection.refreshTargetByParentBox(uiManager.getElement(debugPanelIndex).getBox());
    debugCameraSection.snapToTarget();
    debugCameraSectionIndex = uiManager.addElement(debugCameraSection);
}
// 步骤功能：在一帧最开始时，清空当前所有活跃演员身上上一帧残留的碰撞/重叠/阻挡的标记。
// 这样物理引擎在接下来计算移动时，能拿到一张干干净净的状态纸开始写新的判定。
void Level::clearEntityFrameState()
{
    auto& entities = entityManager.getEntities();
    for (size_t idx : entityManager.getActiveIndices())
    {
        entities[idx].clearFrameState();
    }
}

// 步骤功能：高频物理与动作更新。
// 对大池子里的每一个活着并活跃的演员执行以下五部曲：
// 1. 创建本帧默认的意图（不按键就代表什么都不想做）。
// 2. 如果当前角色正好受键盘控制，由 PlayerController 搜集按键并翻译为意图（如水平想往左、想按下跳跃等）。
// 3. 调用物理引擎 movementHandle.update()，根据意图算速度，并调用 collisionHandle 测算障碍，修正实体坐标。
// 4. 让 Animator 状态机自动根据实体是否在空、在地面、是否在冲刺来决策应该播放哪一类动画（比如 idle 或 jump）。
// 5. 推进当前动画帧计时器，并将最新一帧的裁剪源图贴到实体 Sprite 身上备用。
void Level::updateEntities(InputManager& input)
{
    // 安全控制保护：如果当前操控的主角意外死亡，我们需要默认把操控权恢复给第一个活着的 Player
    Entity* controlTarget = entityManager.getEntity(controlledPlayerId);
    if (!controlTarget || !controlTarget->getIsAlive())
    {
        controlTarget = nullptr;
        for (size_t idx : entityManager.getActiveIndices())
        {
            Entity& ent = entityManager.getEntities()[idx];
            if (ent.getIsAlive() && ent.getEntityType() == PLAYER)
            {
                controlTarget = &ent;
                controlledPlayerId = ent.getId();
                break;
            }
        }
        if (!controlTarget)
        {
            controlledPlayerId = INVALID_ENTITY_ID;
        }
    }

    auto& entities = entityManager.getEntities();
    const auto& activeIndices = entityManager.getActiveIndices();
    for (size_t idx : activeIndices)
    {
        if (!entities[idx].getIsAlive())
        {
            continue; // 忽略死人，防止对无用槽位空转计算
        }

        BehaviorIntent intent;

        // 如果这个角色当前被标记受控，让翻译官去生成意图
        if (entities[idx].isControlled())
        {
            intent = playerController.makeIntent(input, entities[idx].isGod());
        }

        // 送进物理流水线做位置、速度、阻挡修正更新
        movementHandle.update(
            entities[idx],      // 当前更新的角色
            intent,             // 它的意图
            entities,           // 整个大箱子，用来查询跟别的演员有没有物理相撞
            activeIndices,      // 活跃列表
            (int)idx,           // 它自己在对象池里的槽位号
            tileMap,            // 地图图块信息，用来防止穿墙
            worldWidth,         // 关卡世界宽度
            worldHeight,        // 关卡世界高度
            collisionHandle     // 碰撞盒子判定处理器
        );

        // 让 Animator 状态机给它决策本帧动画片段
        entities[idx].updateAnimator(intent);
        
        // 推进精灵图动画的帧更新
        entities[idx].updateAnimatedSprite();
    }
}

// 步骤功能：设置当前被键盘操控的主角。
// 并把其它所有人的操控权一并剥夺。
// name: 想操控的实体 ID 名字（找不到或者死了就不进行任何操作）
void Level::setControlTarget(EntityID targetId)
{
    Entity* target = entityManager.getEntity(targetId);
    if (!target || !target->getIsAlive())
    {
        return; // 安全保护：如果新目标是个空指针或者已经死了，直接拒绝切人
    }

    controlledPlayerId = targetId;

    auto& entities = entityManager.getEntities();
    // 遍历所有活着的人，给被选中的人 setControlled(true)，其它人 setControlled(false)
    for (size_t idx : entityManager.getActiveIndices())
    {
        entities[idx].setControlled(entities[idx].getId() == controlledPlayerId);
    }

    cout << "操控角色已安全切换至 ID: " << controlledPlayerId << " (" << target->getName() << ")" << endl;
}

void Level::setControlTargetByName(const std::string& name)
{
    auto& entities = entityManager.getEntities();
    for (size_t idx : entityManager.getActiveIndices())
    {
        if (entities[idx].getName() == name)
        {
            setControlTarget(entities[idx].getId());
            return;
        }
    }
}

void Level::setCameraFollowTargetByName(const std::string& name)
{
    auto& entities = entityManager.getEntities();
    for (size_t idx : entityManager.getActiveIndices())
    {
        if (entities[idx].getName() == name)
        {
            setCameraFollowTarget(entities[idx].getId(), entityManager);
            return;
        }
    }
}

void Level::handleControlInput(InputManager& input)
{
    if (input.isKeyPressed('1'))
    {
        setControlTargetByName("Player1");
    }

    if (input.isKeyPressed('2'))
    {
        setControlTargetByName("Player2");
    }

    if (input.isKeyPressed('3'))
    {
        setControlTargetByName("Player3");
    }

    if (input.isKeyPressed('4'))
    {
        setControlTargetByName("Player4");
    }
}

void Level::handleCameraInput(InputManager& input)
{
    if (input.isKeyPressed(VK_F1))
    {
        setCameraFollowTargetByName("Player1");
    }

    if (input.isKeyPressed(VK_F2))
    {
        setCameraFollowTargetByName("Player2");
    }

    if (input.isKeyPressed(VK_F3))
    {
        setCameraFollowTargetByName("Player3");
    }

    if (input.isKeyPressed(VK_F4))
    {
        setCameraFollowTargetByName("Player4");
    }

    // 统一处理相机镜头缩放控制（从 CameraFollow 中解耦）
    if (input.isKeyDown('B'))
    {
        gCamera.zoomTo(0.3);
    }
    else if (input.isKeyDown('V'))
    {
        gCamera.zoomTo(3.0);
    }
    else
    {
        gCamera.zoomTo(1.0);
    }
}

void Level::handleUIInput(InputManager& input)
{
    // 以下 F8-F11 仅用于当前阶段验证 Debug UI 层级、显隐状态和 section 动画。
    // 后续正式 UI 交互应改由按钮、配置面板或 Debug 菜单自身控制。
    if (input.isKeyPressed(VK_F8))
    {
        toggleDebugPanelVisible();
        cout << "Toggle debug panel." << endl;
    }

    if (input.isKeyPressed(VK_F9))
    {
        toggleDebugEntitySectionVisible();
        cout << "Toggle debug entity section." << endl;
    }

    if (input.isKeyPressed(VK_F10))
    {
        toggleDebugRenderSectionVisible();
        cout << "Toggle debug render section." << endl;
    }

    if (input.isKeyPressed(VK_F11))
    {
        toggleDebugCameraSectionVisible();
        cout << "Toggle debug camera section." << endl;
    }

}

void Level::toggleDebugPanelVisible()
{
    toggleUIElementVisible(debugPanelIndex);
}

void Level::toggleDebugEntitySectionVisible()
{
    UIElement& element = uiManager.getElement(debugEntitySectionIndex);

    if (element.isVisible())
    {
        uiManager.hideElementAnimatedToOffset(debugEntitySectionIndex, 40, 0);
    }
    else
    {
        uiManager.showElementAnimatedFromOffset(debugEntitySectionIndex, -40, 0);
    }
}

void Level::toggleDebugRenderSectionVisible()
{
    UIElement& element = uiManager.getElement(debugRenderSectionIndex);

    if (element.isVisible())
    {
        uiManager.hideElementAnimatedToOffset(debugRenderSectionIndex, -40, 0);
    }
    else
    {
        uiManager.showElementAnimatedFromOffset(debugRenderSectionIndex, 40, 0);
    }
}

void Level::toggleDebugCameraSectionVisible()
{
    UIElement& element = uiManager.getElement(debugCameraSectionIndex);

    if (element.isVisible())
    {
        uiManager.hideElementAnimatedToOffset(debugCameraSectionIndex, 0, -40);
    }
    else
    {
        uiManager.showElementAnimatedFromOffset(debugCameraSectionIndex, 0, 40);
    }
}

void Level::toggleUIElementVisible(int elementIndex)
{
    if (!uiManager.isValidIndex(elementIndex))
    {
        return;
    }

    UIElement& element = uiManager.getElement(elementIndex);

    if (element.isVisible())
    {
        element.hide();
    }
    else
    {
        uiManager.showElementInstant(elementIndex);
    }
}

void Level::handleRendererInput(InputManager& input)
{
	if (input.isKeyPressed(VK_F5))
	{
		renderer.toggleCollisionBox();
        cout << "Toggle entity collision box." << endl;
	}

	if (input.isKeyPressed(VK_F6))
	{
		renderer.toggleTileCollisionBox();
		cout << "Toggle map tile edge." << endl;

	}
    if (input.isKeyPressed(VK_F7))
    {
        renderer.toggleRenderBounds();
        cout << "Toggle render bounds." << endl;
    }
}

void Level::updateCamera(InputManager& input)
{
    double oldCenterX = gCamera.centerX;
    double oldCenterY = gCamera.centerY;

    updateCameraFollow(
        entityManager,
        worldWidth,
        worldHeight,
        0,0
    );

    gCamera.dx = gCamera.centerX - oldCenterX;
    gCamera.dy = gCamera.centerY - oldCenterY;
}



// 步骤功能：监测各种物理状态转移日志。
// 每帧遍历活着的演员，对比他们本帧和上一帧的状态，一旦发生变化就打印提示。
// 比如：Player1 从空中落到了地上，或者金币 died（死亡了）。
void Level::updateDebugStates()
{
    auto& entities = entityManager.getEntities();
    const auto& activeIndices = entityManager.getActiveIndices();

    for (size_t idx : activeIndices)
    {
        Entity& ent = entities[idx];
        if (!ent.getIsAlive())
        {
            continue; // 忽略死人槽位
        }

        // 碰撞状态监测：如果这帧撞上了，但上帧没撞，打印碰撞开始
        if (ent.hasCollisionState() && !ent.lastCollisionState)
        {
            cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 开始碰到阻挡物了。" << endl;
        }
        ent.lastCollisionState = ent.hasCollisionState(); // 更新历史缓存

        // 落地状态监测：如果这帧站在地上了，但上帧还在空中，打印落地
        if (ent.isOnGround() && !ent.lastGroundState)
        {
            cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 稳稳落地。" << endl;
        }
        ent.lastGroundState = ent.isOnGround();

        // 悬空状态监测：如果起飞悬空了
        if (ent.isInAir() && !ent.lastInAirState)
        {
            cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 处于悬空状态。" << endl;
        }
        ent.lastInAirState = ent.isInAir();

        // 起跳状态监测
        bool nowJumping = ent.isJumping();
        if (nowJumping && !ent.lastJumpingState)
        {
            cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 开始跳跃！" << endl;
        }
        if (!nowJumping && ent.lastJumpingState)
        {
            cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 结束了跳跃。" << endl;
        }
        ent.lastJumpingState = nowJumping;

        // 冲刺/奔跑状态监测
        bool nowSprinting = ent.isSprinting();
        if (nowSprinting && !ent.lastSprintState)
        {
            cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 开始撒丫子狂奔（冲刺）。" << endl;
        }
        if (!nowSprinting && ent.lastSprintState)
        {
            cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 停止了狂奔。" << endl;
        }
        ent.lastSprintState = nowSprinting;
    }

    // 死亡日志监测：
    // 因为这发生在帧末 processSpawns 之前，所以刚死的实体依然在 activeIndices 里。
    // 我们在此捕获它们死亡的瞬间并打印它已经仙逝。
    for (size_t idx : activeIndices)
    {
        Entity& ent = entities[idx];
        bool nowAlive = ent.getIsAlive();
        if (!nowAlive && ent.lastAliveState)
        {
            cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 驾鹤西去（死亡）。" << endl;
        }
        ent.lastAliveState = nowAlive; // 同步生存历史标志
    }
}

// 步骤功能：双重循环检测实体两两之间是否有 AABB 碰撞重叠，并将碰撞事件填充到双方身上。
// 
// 为什么只在 activeIndices 中算？
// 因为这样可以过滤掉死掉的金币或空闲实体，把运算开销从 O(N^2) 降低到 O(M^2)（M 为活人数量，如 10-20）。
// 去重思路：
// 我们用当前帧的名单 currentOverlapPairs 记录本帧发生了的所有碰撞组合（Key 是字母排序后的 ID 拼接，如 "Coin1_Player1"）。
// 如果这个组合在去年的老名单 lastOverlapPairs 中查不到（调用 contains 辅助函数返回 false），
// 说明这是一个“新鲜出炉的碰撞”！我们只在此时打印控制台日志。
void Level::updateOverlapEvents()
{
    auto& entities = entityManager.getEntities();
    const auto& activeIndices = entityManager.getActiveIndices();
    std::vector<std::string> currentOverlapPairs;

    for (size_t i = 0; i < activeIndices.size(); i++)
    {
        for (size_t j = i + 1; j < activeIndices.size(); j++)
        {
            size_t idxA = activeIndices[i];
            size_t idxB = activeIndices[j];

            // 死亡槽位或者不参与碰撞的实体（比如上帝模式的角色）直接跳过
            if (!entities[idxA].getIsAlive() || !entities[idxB].getIsAlive())
            {
                continue;
            }

            if (!entities[idxA].isCollidable() || !entities[idxB].isCollidable())
            {
                continue;
            }

            // 获取两者本帧的世界碰撞盒边界
            RectBox a = entities[idxA].getWorldCollisionBox();
            RectBox b = entities[idxB].getWorldCollisionBox();

            // 调用碰撞助手计算两个矩形是否有重叠相交
            bool overlapping = collisionHandle.isRectOverlapping(a, b);

            if (overlapping)
            {
                // 如果相撞，给两个实体标记上本帧 overlapping 标志（以供绘制红色碰撞盒）
                entities[idxA].setOverlapping(true);
                entities[idxB].setOverlapping(true);

                // 核心：把对方的名字 ID 和类型登记到各自内部 of currentOverlaps vector 中，实现碰撞关系存储
                entities[idxA].addOverlap(entities[idxB].getId(), entities[idxB].getEntityType());
                entities[idxB].addOverlap(entities[idxA].getId(), entities[idxA].getEntityType());

                // 将两者的 ID 按照大小排序拼接成一个唯一的键，避免 (A, B) 和 (B, A) 产生多余判断
                std::string key = (entities[idxA].getId() < entities[idxB].getId()) ?
                                  (std::to_string(entities[idxA].getId()) + "_" + std::to_string(entities[idxB].getId())) :
                                  (std::to_string(entities[idxB].getId()) + "_" + std::to_string(entities[idxA].getId()));
                
                // 登记在当前帧的碰撞名单上
                currentOverlapPairs.push_back(key);

                // 如果上一帧并没有碰过它，才打印日志（防止控制台疯狂刷屏）
                if (!contains(lastOverlapPairs, key))
                {
                    cout << "检测到新重叠事件：实体 [" << entities[idxA].getName() << "] (ID: " << entities[idxA].getId() 
                         << ") 碰到了 实体 [" << entities[idxB].getName() << "] (ID: " << entities[idxB].getId() << ")" << endl;
                }
            }
        }
    }
    // 交接棒：将本帧数据存入历史，留待下一帧作为“上一帧”使用
    lastOverlapPairs = currentOverlapPairs;
}

// 步骤功能：通知活着的实体自己去处理刚才发生的碰撞。
// 因为上一阶段已经把碰到的所有人塞进了 entities[idx].currentOverlaps，
// 这里就是通知每个实体自治响应（金币吃掉自毁、旗帜遇到玩家升旗）。
void Level::resolveEntityOverlaps()
{
    auto& entities = entityManager.getEntities();
    const auto& activeIndices = entityManager.getActiveIndices();
    for (size_t idx : activeIndices)
    {
        if (entities[idx].getIsAlive())
        {
            // 实体自治逻辑
            entities[idx].resolveOverlaps(entityManager);
        }
    }
}
