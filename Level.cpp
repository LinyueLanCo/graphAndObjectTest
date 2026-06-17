#include "Level.h"

#include "Camera.h"

DebugPanelData Level::buildDebugPanelData()
{
    DebugPanelData data;

    data.targetIndex = gCameraFollowTargetIndex;

    if (
        gCameraFollowTargetIndex >= 0 &&
        gCameraFollowTargetIndex < (int)entitys.size()
        )
    {
        Entity& target = entitys[gCameraFollowTargetIndex];

        data.entityX = target.getX();
        data.entityY = target.getY();

        data.entityScreenX = gCamera.worldToScreenX(target.getX());
        data.entityScreenY = gCamera.worldToScreenY(target.getY());
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

    controlTargerIndex = 0;

    // 初始化 UI 元素下标；-1 表示当前还没有创建对应元素。
    debugPanelIndex = -1;

    debugEntitySectionIndex = -1;
    debugRenderSectionIndex = -1;
    debugCameraSectionIndex = -1;

    // 预留当前测试关卡的实体数量，避免初始化期间 vector 扩容搬移 Entity。
    entitys.reserve(20);

    entitys.emplace_back(200, 700, true, true, true, false, PLAYER, ANIM_SET_PLAYER1, 1);

    entitys.emplace_back(_T("assets\\tex\\entities\\characters\\player2.png"), 600, 1300, false, true, false, false, ENTITY, 1);

    entitys.emplace_back(_T("assets\\tex\\entities\\characters\\player3.png"), 950, 1300, false, true, true, false, ENTITY, 1);

    entitys.emplace_back(_T("assets\\tex\\entities\\characters\\player4.png"), 1300, 1300, false, true, false, false, ENTITY, 1);

    entitys.emplace_back(_T("assets\\tex\\entities\\items\\MonedaD.png"), 256, 256, false, true, false, true, COIN, 5, 1);

    entitys.emplace_back(_T("assets\\tex\\entities\\items\\MonedaP.png"), 256 + 48 + 16, 256, false, true, false, true, COIN, 5, 1);

    entitys.emplace_back(_T("assets\\tex\\entities\\items\\MonedaR.png"), 256 + (48 * 2) + (16 * 2), 256, false, true, false, true, COIN, 5, 1);

    entitys.emplace_back(_T("assets\\tex\\entities\\items\\MonedaD.png"), 5662, 1312, false, true, false, true, COIN, 5, 1);

    entitys.emplace_back(_T("assets\\tex\\entities\\items\\MonedaD.png"), 5662, 1312 + 64 + 16, false, true, false, true, COIN, 5, 1);

    entitys.emplace_back(_T("assets\\tex\\entities\\items\\MonedaD.png"), 5662, 1312 + 64 + 16 + 64 + 16, false, true, false, true, COIN, 5, 1);






    worldWidth = WINDOW_WIDTH;
    worldHeight = WINDOW_HEIGHT;

    parallaxCameraX = 0.0;
    parallaxOriginX = 0.0;
}

void Level::initEntityAnimations()
{
    for (int i = 0; i < (int)entitys.size(); i++)
    {
        entitys[i].initAnimationFromAnimator(animationClips);
    }
}

void Level::init()
{
    initResources();

    initMap();
    initBackground();
    initUI();
    initEntityAnimations();
    initEntitySettings();
    initLastStates();

    // 初始化相机位置，避免第一帧背景原点和真实相机位置不同。
    if (!entitys.empty())
    {
        gCamera.followInstant(entitys[0].getX(), entitys[0].getY(), worldWidth, worldHeight);
    }

    parallaxCameraX = getParallaxCameraCenterX();
    parallaxOriginX = parallaxCameraX;
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
    backgroundManager.updateRuntimeTransforms(parallaxOffsetX);

    updateDebugStates();

    updateOverlapEvents();
    resolveEntityOverlaps();
    uiManager.update();
}

void Level::draw()
{
    renderFrameStats.backgroundSpriteCount =
        renderer.drawBackgroundObjects(backgroundManager);
    renderFrameStats.tileSpriteCount =
        renderer.drawTileMap(tileMap);

    renderFrameStats.entitySpriteCount =
        renderer.drawEntities(entitys);

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
    tileMap.loadTileset(_T("assets\\tex\\maps\\tileset.png"));
    tileMap.loadFromFile("assets\\tex\\maps\\map.txt");

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


    backgroundManager.addObjectFromImage2D(
        skyImage,
        0,
        0.11,
        0.0,
        false,
        BACKGROUND_REPEAT_X,
        800,
        450,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );


    backgroundManager.addObjectFromImage2D(
        cloudsImage,
        1,
        0.26,
        0.15,
        true,
        BACKGROUND_REPEAT_X,
        800,
        450,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    backgroundManager.addObjectFromImage2D(
        flora1Image,
        2,
        0.5,
        0.35,
        true,
        BACKGROUND_REPEAT_X,
        800,
        450,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    backgroundManager.addObjectFromImage2D(
        flora2Image,
        3,
        0.73,
        0.65,
        true,
        BACKGROUND_REPEAT_X,
        800,
        450,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
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

void Level::initEntitySettings()
{
    entitys[0].setSpriteTransform(2.0, 2.0, 0, 0);

    entitys[3].setSpriteTransform(2.0, 2.0, 0, 0);

    entitys[4].setSpriteTransform(4.0, 4.0, 0, 0);
    entitys[5].setSpriteTransform(4.0, 4.0, 0, 0);
    entitys[6].setSpriteTransform(4.0, 4.0, 0, 0);
    entitys[7].setSpriteTransform(4.0, 4.0, 0, 0);
    entitys[8].setSpriteTransform(4.0, 4.0, 0, 0);
    entitys[9].setSpriteTransform(4.0, 4.0, 0, 0);



    entitys[0].setAnimationSpeed(3);
    entitys[4].setAnimationSpeed(3);

    entitys[0].setCollisionScale(1.2, 2);
}

void Level::initLastStates()
{
    int entityCount = (int)entitys.size();

    // 根据当前实体数量重建缓存；以后真正 erase 实体后也需要重新同步这些容器。
    lastCollisionState.assign(entityCount, false);
    lastGroundState.assign(entityCount, false);
    lastSprintState.assign(entityCount, false);
    lastInAirState.assign(entityCount, false);
    lastJumpingState.assign(entityCount, false);
    lastAliveState.assign(entityCount, false);
    lastOverlap.assign(entityCount, vector<bool>(entityCount, false));

    for (int i = 0; i < entityCount; i++)
    {
        lastAliveState[i] = entitys[i].getIsAlive();
    }
}

void Level::clearEntityFrameState()
{
    for (int i = 0; i < (int)entitys.size(); i++)
    {
        if (!entitys[i].getIsAlive())
        {
            continue;
        }

        entitys[i].clearFrameState();
    }
}

void Level::updateEntities(InputManager& input)
{
    /*
    实体更新数据流：
        对每个存活实体：
            1. 创建默认空 BehaviorIntent
            2. 如果是玩家 i == 0，则由 PlayerController 根据输入生成 intent
            3. MovementHandle 根据 intent 更新移动/物理
            4. MovementHandle 内部调用 CollisionHandle 进行阻挡修正
            5. Entity 根据 intent 和 sprinting 等状态切换动画
            6. 推进动画帧
    */

    if (
        controlTargerIndex < 0 ||
        controlTargerIndex >= (int)entitys.size() ||
        !entitys[controlTargerIndex].getIsAlive()
        )
    {
        controlTargerIndex = 0;
    }


    for (int i = 0; i < (int)entitys.size(); i++)
    {
        if (!entitys[i].getIsAlive())
        {
            continue;
        }

        BehaviorIntent intent;

        if (
            controlTargerIndex >= 0 &&
            controlTargerIndex < (int)entitys.size() &&
            i == controlTargerIndex
            )
        {
            intent = playerController.makeIntent(input, entitys[i].isGod());
        }
        movementHandle.update(
            entitys[i],
            intent,
            entitys,
            i,
            tileMap,
            worldWidth,
            worldHeight,
            collisionHandle
        );

		entitys[i].updateAnimator(intent, animationClips);
		entitys[i].updateAnimatedSprite();
    }
}

void Level::setControlTarget(int newTargetIndex)
{
    int entityCount = (int)entitys.size();

    if (newTargetIndex < 0 || newTargetIndex >= entityCount)
    {
        return;
    }

    if (!entitys[newTargetIndex].getIsAlive())
    {
        return;
    }

    controlTargerIndex = newTargetIndex;

    cout << "Control target changed to Entity "
        << controlTargerIndex
        << endl;
}

void Level::handleControlInput(InputManager& input)
{
    if (input.isKeyPressed('1'))
    {
        setControlTarget(0);
    }

    if (input.isKeyPressed('2'))
    {
        setControlTarget(1);
    }

    if (input.isKeyPressed('3'))
    {
        setControlTarget(2);
    }

    if (input.isKeyPressed('4'))
    {
        setControlTarget(3);
    }
}

void Level::handleCameraInput(InputManager& input)
{
    if (input.isKeyPressed(VK_F1))
    {
        setCameraFollowTarget(0, entitys);
    }

    if (input.isKeyPressed(VK_F2))
    {
        setCameraFollowTarget(1, entitys);
    }

    if (input.isKeyPressed(VK_F3))
    {
        setCameraFollowTarget(2, entitys);
    }

    if (input.isKeyPressed(VK_F4))
    {
        setCameraFollowTarget(3, entitys);
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
        entitys,
        worldWidth,
        worldHeight,
        //input.getMouseOffsetX(),
        //input.getMouseOffsetY()
        0,0
    );
}

double Level::getParallaxCameraCenterX()
{
    if (
        gCameraFollowTargetIndex < 0 ||
        gCameraFollowTargetIndex >= (int)entitys.size()
        )
    {
        return parallaxCameraX;
    }

    double targetX = entitys[gCameraFollowTargetIndex].getX();

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

void Level::updateParallaxCamera()
{
    double targetParallaxCameraX = getParallaxCameraCenterX();

    double followSpeed = 0.16;
    parallaxCameraX += (targetParallaxCameraX - parallaxCameraX) * followSpeed;
}

void Level::updateDebugStates()
{
    for (int i = 0; i < (int)entitys.size(); i++)
    {
        if (entitys[i].hasCollisionState() && !lastCollisionState[i])
        {
            cout << "Entity " << i << " collision state started." << endl;
        }

        lastCollisionState[i] = entitys[i].hasCollisionState();
    }

    for (int i = 0; i < (int)entitys.size(); i++)
    {
        bool nowAlive = entitys[i].getIsAlive();

        if (!nowAlive && lastAliveState[i])
        {
            cout << "Entity " << i << " died." << endl;
        }

        lastAliveState[i] = nowAlive;
    }

    for (int i = 0; i < (int)entitys.size(); i++)
    {
        if (entitys[i].isOnGround() && !lastGroundState[i])
        {
            cout << "Entity " << i << " is on ground." << endl;
        }

        lastGroundState[i] = entitys[i].isOnGround();
    }

    for (int i = 0; i < (int)entitys.size(); i++)
    {
        if (entitys[i].isInAir() && !lastInAirState[i])
        {
            cout << "Entity " << i << " is in air." << endl;
        }

        lastInAirState[i] = entitys[i].isInAir();
    }

    for (int i = 0; i < (int)entitys.size(); i++)
    {
        bool nowJumping = entitys[i].isJumping();

        if (nowJumping && !lastJumpingState[i])
        {
            cout << "Entity " << i << " started jumping." << endl;
        }

        if (!nowJumping && lastJumpingState[i])
        {
            cout << "Entity " << i << " ended jumping." << endl;
        }

        lastJumpingState[i] = nowJumping;
    }

    for (int i = 0; i < (int)entitys.size(); i++)
    {
        bool nowSprinting = entitys[i].isSprinting();

        if (nowSprinting && !lastSprintState[i])
        {
            cout << "Entity " << i << " started sprinting." << endl;
        }

        if (!nowSprinting && lastSprintState[i])
        {
            cout << "Entity " << i << " ended sprinting." << endl;
        }

        lastSprintState[i] = nowSprinting;
    }
}

void Level::updateOverlapEvents()
{
    /*
    重叠事件检测：
        只负责计算两个 AABB 是否重叠，并将重叠信息填充进双方实体的重叠列表。
    */
    for (int i = 0; i < (int)entitys.size(); i++)
    {
        for (int j = i + 1; j < (int)entitys.size(); j++)
        {
            if (!entitys[i].getIsAlive() || !entitys[j].getIsAlive())
            {
                continue;
            }

            if (!entitys[i].isCollidable() || !entitys[j].isCollidable())
            {
                continue;
            }

            RectBox a = entitys[i].getWorldCollisionBox();
            RectBox b = entitys[j].getWorldCollisionBox();

            bool overlapping = collisionHandle.isRectOverlapping(a, b);

            if (overlapping)
            {
                entitys[i].setOverlapping(true);
                entitys[j].setOverlapping(true);

                // 填充重叠双方实体的重叠列表（记录对方的唯一 ID 和类型）
                entitys[i].addOverlap(entitys[j].getId(), entitys[j].getEntityType());
                entitys[j].addOverlap(entitys[i].getId(), entitys[i].getEntityType());

                if (!lastOverlap[i][j])
                {
                    cout << "Entity ID: " << entitys[i].getId() << " overlaps with Entity ID: " << entitys[j].getId() << endl;
                }
            }

            if (!entitys[i].getIsAlive() || !entitys[j].getIsAlive())
            {
                continue;
            }

            lastOverlap[i][j] = overlapping;
        }
    }
}

void Level::resolveEntityOverlaps()
{
    // 让所有存活实体各自独立响应处理这一帧发生的重叠反馈逻辑
    for (int i = 0; i < (int)entitys.size(); i++)
    {
        if (entitys[i].getIsAlive())
        {
            entitys[i].resolveOverlaps(entitys);
        }
    }
}
