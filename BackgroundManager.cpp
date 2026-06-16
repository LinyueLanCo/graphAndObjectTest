#include "BackgroundManager.h"
#include "Camera.h"

// 功能：获取本帧实际参与绘制的背景对象列表。
vector<BackgroundObject>& BackgroundManager::getRenderObjects()
{
    return renderObjects;
}

// 功能：获取本帧实际参与绘制的背景对象只读列表。
const vector<BackgroundObject>& BackgroundManager::getRenderObjects() const
{
    return renderObjects;
}

// 功能：根据背景对象列表重建本帧实际参与绘制的背景对象列表。
void BackgroundManager::rebuildRenderObjects()
{
    renderObjects.clear();

    for (int i = 0; i < (int)objects.size(); i++)
    {
        BackgroundObject& object = objects[i];

        if (!object.visible)
        {
            continue;
        }

        if (object.drawMode != BACKGROUND_REPEAT_X)
        {
            renderObjects.push_back(object);
            continue;
        }

        double repeatDrawW = object.renderSprite.worldDrawW;
        double repeatDrawH = object.renderSprite.worldDrawH;

        if (repeatDrawW <= 0 || repeatDrawH <= 0)
        {
            continue;
        }

        double viewLeft = gCamera.getViewLeft();
        double viewRight = gCamera.getViewRight();

        double baseCenterX = object.runtimeCenterX;
        double renderCenterY = object.runtimeCenterY;

        int repeatIndex = (int)floor((viewLeft - baseCenterX) / repeatDrawW) - 1;
        double currentCenterX = baseCenterX + repeatIndex * repeatDrawW;

        while (currentCenterX - repeatDrawW / 2.0 <= viewRight + repeatDrawW)
        {
            BackgroundObject repeatedObject = object;

            repeatedObject.generatedByTiling = true;

            repeatedObject.runtimeCenterX = currentCenterX;
            repeatedObject.runtimeCenterY = renderCenterY;

            repeatedObject.renderSprite.setWorldDrawData(
                currentCenterX,
                renderCenterY,
                repeatDrawW,
                repeatDrawH
            );

            renderObjects.push_back(repeatedObject);

            currentCenterX += repeatDrawW;
        }
    }

    sort(
        renderObjects.begin(),
        renderObjects.end(),
        [](const BackgroundObject& a, const BackgroundObject& b)
        {
            return a.renderOrder < b.renderOrder;
        }
    );
}

// 功能：清空所有背景对象和本帧绘制对象。
void BackgroundManager::clear()
{
    objects.clear();
    renderObjects.clear();
}

// 功能：根据已加载的图片资源创建一个背景对象。
void BackgroundManager::addObjectFromImage2D(
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
)
{
    if (imageResource == NULL)
    {
        return;
    }

    BackgroundObject object;

    object.setRenderData(
        newRenderOrder,
        newParallaxFactor,
        newZoomFactor,
        newUseAlphaBlend,
        newDrawMode
    );

    object.setDrawData(
        newCenterX,
        newCenterY,
        newDrawW,
        newDrawH
    );

    object.bindSpriteSource(imageResource);
    object.updateRuntimeTransform(0.0);
    object.updateSprite();

    objects.push_back(object);
    sortObjectsByRenderOrder();
    rebuildRenderObjects();
}

// 功能：更新所有背景对象本帧用于生成 sprite 的逻辑变换。
void BackgroundManager::updateRuntimeTransforms(double parallaxOffsetX)
{
    for (int i = 0; i < (int)objects.size(); i++)
    {
        objects[i].updateRuntimeTransform(parallaxOffsetX);
        objects[i].updateSprite();
    }

    rebuildRenderObjects();
}

// 功能：按照 renderOrder 从小到大排序背景对象，数值越小越先绘制。
void BackgroundManager::sortObjectsByRenderOrder()
{
    sort(
        objects.begin(),
        objects.end(),
        [](const BackgroundObject& a, const BackgroundObject& b)
        {
            return a.renderOrder < b.renderOrder;
        }
    );
}

// 功能：清空所有背景对象实例。
void BackgroundManager::clearObjects()
{
    objects.clear();
}

// 功能：添加一个背景对象实例，并按 renderOrder 维护绘制顺序。
void BackgroundManager::addObject(const BackgroundObject& object)
{
    objects.push_back(object);
    sortObjectsByRenderOrder();
}

// 功能：获取背景对象实例数量。
int BackgroundManager::getObjectCount() const
{
    return (int)objects.size();
}

// 功能：获取背景对象可写列表。
vector<BackgroundObject>& BackgroundManager::getObjects()
{
    return objects;
}

// 功能：获取背景对象只读列表。
const vector<BackgroundObject>& BackgroundManager::getObjects() const
{
    return objects;
}
