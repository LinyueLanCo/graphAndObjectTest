#include "Level.h"
#include "json.hpp"

#include "Camera.h"

DebugPanelData Level::buildDebugPanelData()
{
    DebugPanelData data;

    data.targetId = gCameraFollowTargetId;

    Entity* target = entityManager.getEntityById(gCameraFollowTargetId);
    if (target)
    {
        data.entityX = target->getX();
        data.entityY = target->getY();

        data.entityScreenX = gCamera.worldToScreenX(target->getX());
        data.entityScreenY = gCamera.worldToScreenY(target->getY());
    }

    data.renderedBackgroundSprites = renderFrameStats.backgroundSpriteCount;
    data.renderedTileSprites = renderFrameStats.tileSpriteCount;
    data.renderedEntitySprites = renderFrameStats.entitySpriteCount;
    data.renderedTotalSprites = renderFrameStats.totalSpriteCount;

    data.cameraCenterX = gCamera.centerX;
    data.cameraCenterY = gCamera.centerY;
    data.cameraZoom = gCamera.zoom;

    data.viewLeft = gCamera.getViewLeft();
    data.viewRight = gCamera.getViewRight();
    data.viewBottom = gCamera.getViewBottom();
    data.viewTop = gCamera.getViewTop();

    return data;
}

Level::Level()
{
    controlledEntityName = "Player1";

    // 初始化 UI 元素下标；-1 表示当前还没有创建对应元素。
    debugPanelIndex = -1;

    debugEntitySectionIndex = -1;
    debugRenderSectionIndex = -1;
    debugCameraSectionIndex = -1;

    worldWidth = WINDOW_WIDTH;
    worldHeight = WINDOW_HEIGHT;

    parallaxCameraX = 0.0;
    parallaxCameraY = 0.0;
    parallaxOriginX = 0.0;
    parallaxOriginY = 0.0;
}

void Level::initEntityAnimations()
{
    for (auto& ent : entityManager.getEntities())
    {
        ent.initAnimationFromAnimator(animationClips);
    }
}

void Level::init()
{
    initResources();

    initMap();

    // 从 JSON 配置文件加载并初始化实体
    entityManager.loadEntities("assets/data/entities.json", animationClips);

    initUI();

    // 初始化相机位置，避免第一帧背景原点和真实相机位置不同。
    auto& entities = entityManager.getEntities();
    if (!entities.empty())
    {
        gCamera.followInstant(entities[0].getX(), entities[0].getY(), worldWidth, worldHeight);
    }

    parallaxCameraX = getParallaxCameraCenterX();
    parallaxCameraY = getParallaxCameraCenterY();
    parallaxOriginX = parallaxCameraX;
    parallaxOriginY = parallaxCameraY;

    // 同步实体的初始控制状态
    setControlTarget(controlledEntityName);

    initBackground();
}

void Level::update(InputManager& input)
{
    /*
    Level 每帧更新顺序：
        1. 处理输入反馈（临时鼠标调试）
        2. clearEntityFrameState() 清理上一帧临时状态
        3. updateEntities() 生成 intent，并调用 MovementHandle / CollisionHandle
        4. handleCameraInput() 处理 F1-F4 跟随目标切换
        5. handleUIInput() 处理 Debug UI 临时显隐输入
        6. updateCamera() 更新摄像机跟随
        7. updateDebugStates() 输出状态变化
        8. updateOverlapEvents() 处理重叠事件，如金币拾取
        9. uiManager.update() 更新 UIElement 父子定位和过渡
    */
    if (input.isMouseLeftPressed())
    {
        cout << "Left mouse down: "
            << input.getMouseX()
            << " "
            << input.getMouseY()
            << endl;
    }

    clearEntityFrameState();

    handleControlInput(input);

    updateEntities(input);

    handleCameraInput(input);
    handleUIInput(input);
    handleRendererInput(input);

    updateCamera(input);
    updateParallaxCamera();

    double parallaxOffsetX = parallaxCameraX - parallaxOriginX;
    // Y 方向直接使用真实相机中心变化量，避免背景抢在 followSmooth 相机之前移动。
    double parallaxOffsetY = gCamera.centerY - parallaxOriginY;
    backgroundManager.updateRuntimeTransforms(parallaxOffsetX, parallaxOffsetY);

    updateDebugStates();

    updateOverlapEvents();
    resolveEntityOverlaps();

    // 检测旗帜是否刚刚完成升旗动画，并在逻辑中心上方 64 像素生成一个 Coin2 对象
    static int spawnedCoinCounter = 1;
    for (auto& ent : entityManager.getEntities())
    {
        if (ent.getEntityType() == CHECKPOINT && ent.flagActivatedJustNow)
        {
            ent.flagActivatedJustNow = false; // 重置标记

            std::string coinId = "SpawnedCoin_" + std::to_string(spawnedCoinCounter++);
            entityManager.queueSpawnEntity(
                coinId,
                ent.getX(),
                ent.getY() - 64.0,
                false,                 // controlled
                true,                  // collidable
                false,                 // blocking
                true,                  // god
                COIN,                  // type
                ANIM_SET_COIN_SILVER,  // animSet (Coin2)
                4.0, 4.0,              // scaleX, scaleY
                1.0, 1.0,              // colScaleX, colScaleY
                3                      // animSpeed
            );
        }
    }

    // 帧末安全执行所有的动态实体生成
    entityManager.processSpawns(animationClips);

    uiManager.update();
}

void Level::draw()
{
    renderFrameStats.backgroundSpriteCount =
        renderer.drawBackgroundObjects(backgroundManager);
    renderFrameStats.tileSpriteCount =
        renderer.drawTileMap(tileMap);

    renderFrameStats.entitySpriteCount =
        renderer.drawEntities(entityManager.getEntities());

    renderFrameStats.refreshTotal();

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
    animationClips.init(resources);
}

void Level::initMap()
{
    tileMap.setTileSize(16, 16, 48, 48);
    tileMap.loadTileset(resources.getImage2D(IMG_TILESET_MAIN));
    tileMap.loadFromText(resources.getTextContent(TXT_MAP_MAIN));

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

    Image2D* skyImage = resources.getImage2D(IMG_BG_SKY);
    Image2D* cloudsImage = resources.getImage2D(IMG_BG_CLOUDS);
    Image2D* flora1Image = resources.getImage2D(IMG_BG_FLORA1);
    Image2D* flora2Image = resources.getImage2D(IMG_BG_FLORA2);

    double backgroundCenterX = parallaxOriginX;
    double backgroundCenterY = parallaxOriginY;

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
        cloudsImage != NULL ? cloudsImage->getHeight() : WINDOW_HEIGHT
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
void Level::clearEntityFrameState()
{
    for (auto& ent : entityManager.getEntities())
    {
        if (!ent.getIsAlive())
        {
            continue;
        }

        ent.clearFrameState();
    }
}

void Level::updateEntities(InputManager& input)
{
    /*
    实体更新数据流：
        对每个存活实体：
            1. 创建默认空 BehaviorIntent
            2. 如果是当前玩家控制的对象，则由 PlayerController 根据输入生成 intent
            3. MovementHandle 根据 intent 更新移动/物理
            4. MovementHandle 内部调用 CollisionHandle 进行阻挡修正
            5. Entity 根据 intent 和 sprinting 等状态切换动画
            6. 推进动画帧
    */

    Entity* controlTarget = entityManager.getEntityById(controlledEntityName);
    if (!controlTarget || !controlTarget->getIsAlive())
    {
        // 尝试默认恢复到 Player1
        controlTarget = entityManager.getEntityById("Player1");
        if (controlTarget && controlTarget->getIsAlive())
        {
            controlledEntityName = "Player1";
        }
        else
        {
            controlledEntityName = "";
        }
    }

    auto& entities = entityManager.getEntities();
    for (size_t i = 0; i < entities.size(); i++)
    {
        if (!entities[i].getIsAlive())
        {
            continue;
        }

        BehaviorIntent intent;

        if (entities[i].isControlled())
        {
            intent = playerController.makeIntent(input, entities[i].isGod());
        }

        movementHandle.update(
            entities[i],
            intent,
            entities,
            (int)i,
            tileMap,
            worldWidth,
            worldHeight,
            collisionHandle
        );

        entities[i].updateAnimator(intent, animationClips);
        entities[i].updateAnimatedSprite();
    }
}

void Level::setControlTarget(const std::string& name)
{
    Entity* target = entityManager.getEntityById(name);
    if (!target || !target->getIsAlive())
    {
        return;
    }

    controlledEntityName = name;

    // 动态同步每个实体的控制状态
    for (auto& ent : entityManager.getEntities())
    {
        ent.setControlled(ent.getId() == controlledEntityName);
    }

    cout << "Control target changed to Entity "
        << controlledEntityName
        << endl;
}

void Level::handleControlInput(InputManager& input)
{
    if (input.isKeyPressed('1'))
    {
        setControlTarget("Player1");
    }

    if (input.isKeyPressed('2'))
    {
        setControlTarget("Player2");
    }

    if (input.isKeyPressed('3'))
    {
        setControlTarget("Player3");
    }

    if (input.isKeyPressed('4'))
    {
        setControlTarget("Player4");
    }
}

void Level::handleCameraInput(InputManager& input)
{
    if (input.isKeyPressed(VK_F1))
    {
        setCameraFollowTarget("Player1", entityManager);
    }

    if (input.isKeyPressed(VK_F2))
    {
        setCameraFollowTarget("Player2", entityManager);
    }

    if (input.isKeyPressed(VK_F3))
    {
        setCameraFollowTarget("Player3", entityManager);
    }

    if (input.isKeyPressed(VK_F4))
    {
        setCameraFollowTarget("Player4", entityManager);
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
    updateCameraFollow(
        entityManager,
        worldWidth,
        worldHeight,
        0,0
    );
}

double Level::getParallaxCameraCenterX()
{
    Entity* target = entityManager.getEntityById(gCameraFollowTargetId);
    if (!target)
    {
        return parallaxCameraX;
    }

    double targetX = target->getX();

    // 视差背景的横向锚点使用 zoom = 1 时的窗口宽度作为参考视口。
    double referenceVisibleW = WINDOW_WIDTH;
    double referenceHalfW = referenceVisibleW / 2.0;

    if (worldWidth <= referenceVisibleW)
    {
        return worldWidth / 2.0;
    }

    if (targetX < referenceHalfW)
    {
        return referenceHalfW;
    }

    if (targetX > worldWidth - referenceHalfW)
    {
        return worldWidth - referenceHalfW;
    }

    return targetX;
}

double Level::getParallaxCameraCenterY()
{
    Entity* target = entityManager.getEntityById(gCameraFollowTargetId);
    if (!target)
    {
        return parallaxCameraY;
    }

    double targetY = target->getY();

    // 视差背景的纵向锚点使用 zoom = 1 时的窗口高度作为参考视口。
    double referenceVisibleH = WINDOW_HEIGHT;
    double referenceHalfH = referenceVisibleH / 2.0;

    if (worldHeight <= referenceVisibleH)
    {
        return worldHeight / 2.0;
    }

    if (targetY < referenceHalfH)
    {
        return referenceHalfH;
    }

    if (targetY > worldHeight - referenceHalfH)
    {
        return worldHeight - referenceHalfH;
    }

    return targetY;
}

void Level::updateParallaxCamera()
{
    double targetParallaxCameraX = getParallaxCameraCenterX();
    double targetParallaxCameraY = getParallaxCameraCenterY();

    double followSpeed = 0.16;
    parallaxCameraX += (targetParallaxCameraX - parallaxCameraX) * followSpeed;
    parallaxCameraY += (targetParallaxCameraY - parallaxCameraY) * followSpeed;
}

void Level::updateDebugStates()
{
    for (auto& ent : entityManager.getEntities())
    {
        if (!ent.getIsAlive())
        {
            continue;
        }

        // Collision State
        if (ent.hasCollisionState() && !ent.lastCollisionState)
        {
            cout << "Entity " << ent.getId() << " collision state started." << endl;
        }
        ent.lastCollisionState = ent.hasCollisionState();

        // On Ground State
        if (ent.isOnGround() && !ent.lastGroundState)
        {
            cout << "Entity " << ent.getId() << " is on ground." << endl;
        }
        ent.lastGroundState = ent.isOnGround();

        // In Air State
        if (ent.isInAir() && !ent.lastInAirState)
        {
            cout << "Entity " << ent.getId() << " is in air." << endl;
        }
        ent.lastInAirState = ent.isInAir();

        // Jumping State
        bool nowJumping = ent.isJumping();
        if (nowJumping && !ent.lastJumpingState)
        {
            cout << "Entity " << ent.getId() << " started jumping." << endl;
        }
        if (!nowJumping && ent.lastJumpingState)
        {
            cout << "Entity " << ent.getId() << " ended jumping." << endl;
        }
        ent.lastJumpingState = nowJumping;

        // Sprinting State
        bool nowSprinting = ent.isSprinting();
        if (nowSprinting && !ent.lastSprintState)
        {
            cout << "Entity " << ent.getId() << " started sprinting." << endl;
        }
        if (!nowSprinting && ent.lastSprintState)
        {
            cout << "Entity " << ent.getId() << " ended sprinting." << endl;
        }
        ent.lastSprintState = nowSprinting;
    }

    // Dying Log
    for (auto& ent : entityManager.getEntities())
    {
        bool nowAlive = ent.getIsAlive();
        if (!nowAlive && ent.lastAliveState)
        {
            cout << "Entity " << ent.getId() << " died." << endl;
        }
        ent.lastAliveState = nowAlive;
    }
}

void Level::updateOverlapEvents()
{
    /*
    重叠事件检测：
        只负责计算两个 AABB 是否重叠，并将重叠信息填充进双方实体的重叠列表。
    */
    auto& entities = entityManager.getEntities();
    std::unordered_set<std::string> currentOverlapPairs;

    for (size_t i = 0; i < entities.size(); i++)
    {
        for (size_t j = i + 1; j < entities.size(); j++)
        {
            if (!entities[i].getIsAlive() || !entities[j].getIsAlive())
            {
                continue;
            }

            if (!entities[i].isCollidable() || !entities[j].isCollidable())
            {
                continue;
            }

            RectBox a = entities[i].getWorldCollisionBox();
            RectBox b = entities[j].getWorldCollisionBox();

            bool overlapping = collisionHandle.isRectOverlapping(a, b);

            if (overlapping)
            {
                entities[i].setOverlapping(true);
                entities[j].setOverlapping(true);

                // 填充重叠双方实体的重叠列表（记录对方的唯一 ID 和类型）
                entities[i].addOverlap(entities[j].getId(), entities[j].getEntityType());
                entities[j].addOverlap(entities[i].getId(), entities[i].getEntityType());

                std::string key = (entities[i].getId() < entities[j].getId()) ?
                                  (entities[i].getId() + "_" + entities[j].getId()) :
                                  (entities[j].getId() + "_" + entities[i].getId());
                currentOverlapPairs.insert(key);

                if (entityManager.lastOverlapPairs.find(key) == entityManager.lastOverlapPairs.end())
                {
                    cout << "Entity ID: " << entities[i].getId() << " overlaps with Entity ID: " << entities[j].getId() << endl;
                }
            }
        }
    }
    entityManager.lastOverlapPairs = currentOverlapPairs;
}

void Level::resolveEntityOverlaps()
{
    auto& entities = entityManager.getEntities();
    // 让所有存活实体各自独立响应处理这一帧发生的重叠反馈逻辑
    for (size_t i = 0; i < entities.size(); i++)
    {
        if (entities[i].getIsAlive())
        {
            entities[i].resolveOverlaps(entities, animationClips);
        }
    }
}
