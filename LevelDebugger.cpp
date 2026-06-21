#include "LevelDebugger.h"
#include "Camera.h"
#include "CameraFollow.h"
#include "Config.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>

LevelDebugger::LevelDebugger()
{
    debugPanelIndex = -1;
    debugEntitySectionIndex = -1;
    debugRenderSectionIndex = -1;
    debugCameraSectionIndex = -1;
}

void LevelDebugger::init(UIManager& uiManager)
{
    // Debug 面板是顶层 UI，相对于窗口右上角定位。
    UIElement debugPanel;
    debugPanel.init(420, 520, UI_TOP_RIGHT, 24, 24);

    debugPanelIndex = uiManager.addElement(debugPanel);

    // Debug Entity 区域：显示当前相机跟随目标实体的数据。
    UIElement debugEntitySection;
    debugEntitySection.init(388, 130, UI_TOP_LEFT, 16, 16);
    debugEntitySection.setParentIndex(debugPanelIndex);
    debugEntitySection.refreshTargetByParentBox(uiManager.getElement(debugPanelIndex).getBox());
    debugEntitySection.snapToTarget();
    debugEntitySectionIndex = uiManager.addElement(debugEntitySection);

    // Debug Render 区域：显示当前渲染相关数据。
    UIElement debugRenderSection;
    debugRenderSection.init(388, 130, UI_TOP_LEFT, 16, 160);
    debugRenderSection.setParentIndex(debugPanelIndex);
    debugRenderSection.refreshTargetByParentBox(uiManager.getElement(debugPanelIndex).getBox());
    debugRenderSection.snapToTarget();
    debugRenderSectionIndex = uiManager.addElement(debugRenderSection);

    // Debug Camera 区域：显示当前相机和视口数据。
    UIElement debugCameraSection;
    debugCameraSection.init(388, 180, UI_TOP_LEFT, 16, 310);
    debugCameraSection.setParentIndex(debugPanelIndex);
    debugCameraSection.refreshTargetByParentBox(uiManager.getElement(debugPanelIndex).getBox());
    debugCameraSection.snapToTarget();
    debugCameraSectionIndex = uiManager.addElement(debugCameraSection);
}



void LevelDebugger::toggleUIElementVisible(UIManager& uiManager, int elementIndex)
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

void LevelDebugger::toggleDebugEntitySectionVisible(UIManager& uiManager)
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

void LevelDebugger::toggleDebugRenderSectionVisible(UIManager& uiManager)
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

void LevelDebugger::toggleDebugCameraSectionVisible(UIManager& uiManager)
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

void LevelDebugger::handleInput(
    InputManager& input,
    Renderer& renderer,
    UIManager& uiManager,
    EntityManager& entityManager
)
{
    // F5-F7: 开关渲染相关的调试选项
    if (input.isKeyPressed(VK_F5))
    {
        renderer.toggleCollisionBox();
        std::cout << "Toggle entity collision box." << std::endl;
    }

    if (input.isKeyPressed(VK_F6))
    {
        renderer.toggleTileCollisionBox();
        std::cout << "Toggle map tile edge." << std::endl;
    }

    if (input.isKeyPressed(VK_F7))
    {
        renderer.toggleRenderBounds();
        std::cout << "Toggle render bounds." << std::endl;
    }

    // F8-F11: 控制调试UI面板及各个分块的显隐
    if (input.isKeyPressed(VK_F8))
    {
        toggleUIElementVisible(uiManager, debugPanelIndex);
        std::cout << "Toggle debug panel." << std::endl;
    }

    if (input.isKeyPressed(VK_F9))
    {
        toggleDebugEntitySectionVisible(uiManager);
        std::cout << "Toggle debug entity section." << std::endl;
    }

    if (input.isKeyPressed(VK_F10))
    {
        toggleDebugRenderSectionVisible(uiManager);
        std::cout << "Toggle debug render section." << std::endl;
    }

    if (input.isKeyPressed(VK_F11))
    {
        toggleDebugCameraSectionVisible(uiManager);
        std::cout << "Toggle debug camera section." << std::endl;
    }
}

void LevelDebugger::updateDebugLogs(EntityManager& entityManager)
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
            std::cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 开始碰到阻挡物了。" << std::endl;
        }
        ent.lastCollisionState = ent.hasCollisionState(); // 更新历史缓存

        // 落地状态监测：如果这帧站在地上了，但上帧还在空中，打印落地
        if (ent.isOnGround() && !ent.lastGroundState)
        {
            std::cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 稳稳落地。" << std::endl;
        }
        ent.lastGroundState = ent.isOnGround();

        // 悬空状态监测：如果起飞悬空了
        if (ent.isInAir() && !ent.lastInAirState)
        {
            std::cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 处于悬空状态。" << std::endl;
        }
        ent.lastInAirState = ent.isInAir();

        // 起跳状态监测
        bool nowJumping = ent.isJumping();
        if (nowJumping && !ent.lastJumpingState)
        {
            std::cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 开始跳跃！" << std::endl;
        }
        if (!nowJumping && ent.lastJumpingState)
        {
            std::cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 结束了跳跃。" << std::endl;
        }
        ent.lastJumpingState = nowJumping;

        // 冲刺/奔跑状态监测
        bool nowSprinting = ent.isSprinting();
        if (nowSprinting && !ent.lastSprintState)
        {
            std::cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 开始撒丫子狂奔（冲刺）。" << std::endl;
        }
        if (!nowSprinting && ent.lastSprintState)
        {
            std::cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 停止了狂奔。" << std::endl;
        }
        ent.lastSprintState = nowSprinting;
    }

    // 死亡日志监测
    for (size_t idx : activeIndices)
    {
        Entity& ent = entities[idx];
        bool nowAlive = ent.getIsAlive();
        if (!nowAlive && ent.lastAliveState)
        {
            std::cout << "演员 [" << ent.getName() << "] (ID: " << ent.getId() << ") 驾鹤西去（死亡）。" << std::endl;
        }
        ent.lastAliveState = nowAlive; // 同步生存历史标志
    }
}

void LevelDebugger::draw(
    Renderer& renderer,
    UIManager& uiManager,
    EntityManager& entityManager,
    const RenderFrameStats& renderFrameStats
)
{
    // Debug 面板父级负责整体背景；子 section 的最终可见性由 UIManager 按父级链路判断。
    if (uiManager.isElementEffectivelyVisible(debugPanelIndex))
    {
        renderer.drawUIElementPanel(uiManager.getElement(debugPanelIndex));
    }

    char buf[128];

    if (uiManager.isElementEffectivelyVisible(debugEntitySectionIndex))
    {
        std::vector<std::string> lines;
        lines.push_back("Debug Target");

        Entity* target = entityManager.getEntity(gCameraFollowTargetId);
        if (target)
        {
            sprintf_s(buf, "Entity Name: %s", target->getName().c_str());
            lines.push_back(buf);

            sprintf_s(buf, "Entity IID: %d", target->getId());
            lines.push_back(buf);

            sprintf_s(buf, "World Pos: %.1f, %.1f", target->getX(), target->getY());
            lines.push_back(buf);

            sprintf_s(buf, "Screen Pos: %d, %d", (int)gCamera.worldToScreenX(target->getX()), (int)gCamera.worldToScreenY(target->getY()));
            lines.push_back(buf);

            sprintf_s(buf, "Anim State: %s", target->getAnimator().getCurrentState().c_str());
            lines.push_back(buf);
        }
        else
        {
            lines.push_back("No target entity");
        }

        renderer.drawDebugSectionText(uiManager.getElement(debugEntitySectionIndex), lines);
    }

    if (uiManager.isElementEffectivelyVisible(debugRenderSectionIndex))
    {
        std::vector<std::string> lines;
        lines.push_back("Render");

        sprintf_s(buf, "Bg Sprites: %d", renderFrameStats.backgroundSpriteCount);
        lines.push_back(buf);

        sprintf_s(buf, "Tile Sprites: %d", renderFrameStats.tileSpriteCount);
        lines.push_back(buf);

        sprintf_s(buf, "Entity Sprites: %d", renderFrameStats.entitySpriteCount);
        lines.push_back(buf);

        sprintf_s(buf, "Total Sprites: %d", renderFrameStats.totalSpriteCount);
        lines.push_back(buf);

        renderer.drawDebugSectionText(uiManager.getElement(debugRenderSectionIndex), lines);
    }

    if (uiManager.isElementEffectivelyVisible(debugCameraSectionIndex))
    {
        std::vector<std::string> lines;
        lines.push_back("Camera");

        sprintf_s(buf, "Center: %.1f, %.1f", gCamera.centerX, gCamera.centerY);
        lines.push_back(buf);

        sprintf_s(buf, "Zoom: %.2f", gCamera.zoom);
        lines.push_back(buf);

        sprintf_s(buf, "View L/R: %.1f / %.1f", gCamera.getViewLeft(), gCamera.getViewRight());
        lines.push_back(buf);

        sprintf_s(buf, "View B/T: %.1f / %.1f", gCamera.getViewBottom(), gCamera.getViewTop());
        lines.push_back(buf);

        renderer.drawDebugSectionText(uiManager.getElement(debugCameraSectionIndex), lines);
    }
}
