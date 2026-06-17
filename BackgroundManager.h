#pragma once

#include "BackgroundObject.h"
#include "Config.h"

// BackgroundManager:
// 管理当前关卡中的背景对象。
// objects 保存手动创建的背景对象，renderObjects 保存本帧平铺展开后真正参与绘制的背景对象。
class BackgroundManager
{
private:
    vector<BackgroundObject> objects;
    vector<BackgroundObject> renderPool;                 // 预分配的背景渲染对象池，避免每一帧 push_back 造成动态内存扩容和结构体拷贝
    int activeRenderCount;                               // 本帧当前激活并参与实际绘制的背景图块实例数量

public:
    BackgroundManager();                                 // 构造函数：预置渲染池的大小

    vector<BackgroundObject>& getRenderObjects();
    const vector<BackgroundObject>& getRenderObjects() const;
    int getActiveRenderCount() const;                    // 获取当前活跃的背景实例总数

    void rebuildRenderObjects();
    void clear();
    void addObjectFromImage2D(
        Image2D* imageResource,
        int newRenderOrder,
        double newParallaxFactor,
        double newZoomFactor,
        bool newUseAlphaBlend,
        BackgroundDrawMode newDrawMode,
        double newCenterX,
        double newCenterY,
        double newDrawW,
        double newDrawH,
        double newAutoScrollSpeedX = 0.0
    );
    void updateRuntimeTransforms(double cameraVx, double cameraVy);
    void sortObjectsByRenderOrder();
    void clearObjects();
    void addObject(const BackgroundObject& object);
    int getObjectCount() const;
    vector<BackgroundObject>& getObjects();
    const vector<BackgroundObject>& getObjects() const;
};
