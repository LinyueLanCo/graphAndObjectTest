#pragma once

#include "EntityManager.h"
#include "UI.h"
#include "Renderer.h"
#include "Input.h"

// LevelDebugger:
// 负责所有调试面板数据的收集、Debug UI的开关、实体运动日志打印以及调试框层绘制协调。
class LevelDebugger
{
private:
    int debugPanelIndex;
    int debugEntitySectionIndex;
    int debugRenderSectionIndex;
    int debugCameraSectionIndex;



    void toggleUIElementVisible(UIManager& uiManager, int elementIndex);
    void toggleDebugEntitySectionVisible(UIManager& uiManager);
    void toggleDebugRenderSectionVisible(UIManager& uiManager);
    void toggleDebugCameraSectionVisible(UIManager& uiManager);

public:
    LevelDebugger();

    void init(UIManager& uiManager);
    void handleInput(InputManager& input, Renderer& renderer, UIManager& uiManager, EntityManager& entityManager);
    void updateDebugLogs(EntityManager& entityManager);
    void draw(Renderer& renderer, UIManager& uiManager, EntityManager& entityManager, const RenderFrameStats& renderFrameStats);
};
