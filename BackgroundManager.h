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
    vector<BackgroundObject> renderObjects;

public:
    vector<BackgroundObject>& getRenderObjects();
    const vector<BackgroundObject>& getRenderObjects() const;

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
        double newDrawH
    );
    void updateRuntimeTransforms(double parallaxOffsetX);
    void sortObjectsByRenderOrder();
    void clearObjects();
    void addObject(const BackgroundObject& object);
    int getObjectCount() const;
    vector<BackgroundObject>& getObjects();
    const vector<BackgroundObject>& getObjects() const;
};
