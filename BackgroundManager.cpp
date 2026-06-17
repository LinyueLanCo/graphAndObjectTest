#include "BackgroundManager.h"
#include "Camera.h"

// 构造函数：预置渲染对象池的大小，防止运行时频繁扩容拷贝，并将活跃渲染计数初始化为 0。
BackgroundManager::BackgroundManager()
{
    // 预分配 32 个背景槽位，通常足够覆盖多图层的平铺需求，避免每帧动态分配内存导致的卡顿（堆碎片）
    renderPool.resize(32);
    activeRenderCount = 0;
}

// 功能：获取当前帧内经过平铺裁剪后，实际需要提交给渲染器的背景实例列表。
vector<BackgroundObject>& BackgroundManager::getRenderObjects()
{
    return renderPool;
}

// 功能：获取当前帧内实际参与绘制的背景实例只读列表。
const vector<BackgroundObject>& BackgroundManager::getRenderObjects() const
{
    return renderPool;
}

// 功能：获取当前帧内在可视范围里被激活并绘制的背景瓦片总数。
int BackgroundManager::getActiveRenderCount() const
{
    return activeRenderCount;
}

// 核心功能：根据已配置的背景模板列表，结合当前摄像机的可视范围，动态重建并平铺当前帧需要渲染的图块列表。
// 
// 💡 平铺与视口裁剪算法详解：
// 对于普通单张世界背景或跟随相机的背景，我们直接放入渲染队列。
// 对于需要在水平方向上无限平铺（BACKGROUND_REPEAT_X）的背景，我们通过“逻辑视口”和“背景边缘”的几何差值进行常数级平铺：
// 
// 1. 我们获取当前摄像机在世界坐标系中的“逻辑左边缘(viewLeft)”和“逻辑右边缘(viewRight)”。
// 2. 利用 (viewLeft - baseLeftX) 算出现实中屏幕左边界与背景图左边缘的真实空缺像素距离。
// 3. 将其除以图块宽度 repeatDrawW，得到所需补齐的瓦片张数（带小数）。
// 4. 使用 floor() 函数向下取整（向左侧退格），计算得到覆盖屏幕左边界所需要的首个瓦片网格索引(repeatIndex)。
//    - 该公式在数学上极为纯粹且天然对称，无需多余的安全偏移修正（如 -1）。
// 5. 确定起点位置后，我们直接向右每次步进一个完整图宽(repeatDrawW)进行循环平铺填充，直到超过屏幕右侧为止。
void BackgroundManager::rebuildRenderObjects()
{
    // 每帧重新计算前，先重置活跃图块计数器
    activeRenderCount = 0;

    for (int i = 0; i < (int)objects.size(); i++)
    {
        BackgroundObject& object = objects[i];

        // 如果该背景图层被设为隐藏，直接跳过
        if (!object.visible)
        {
            continue;
        }

        // 如果不需要横向平铺，直接将其作为单一实体加入渲染池
        if (object.drawMode != BACKGROUND_REPEAT_X)
        {
            if (activeRenderCount < (int)renderPool.size())
            {
                renderPool[activeRenderCount++] = object;
            }
            continue;
        }

        // 获取背景图在世界场景中的绘制宽高
        double repeatDrawW = object.renderSprite.worldDrawW;
        double repeatDrawH = object.renderSprite.worldDrawH;

        // 若尺寸无效，无法平铺，直接跳过
        if (repeatDrawW <= 0 || repeatDrawH <= 0)
        {
            continue;
        }

        // 获取当前摄像机在世界场景中的逻辑视口左右边界
        double viewLeft = gCamera.getViewLeft();
        double viewRight = gCamera.getViewRight();

        // 1. 计算背景模板图的左边缘世界坐标 (baseLeftX = 逻辑中心点 - 半宽)
        double baseLeftX = object.runtimeCenterX - repeatDrawW / 2.0;
        double renderCenterY = object.runtimeCenterY;

        // 2. 网格坐标投影定位：
        //    - (viewLeft - baseLeftX)：算出了从当前背景左边缘到视口左边缘的物理距离空缺。
        //    - / repeatDrawW：将物理空缺距离转换成“需要补多少张背景图”。
        //    - floor(...)：向下取整（向数轴左方退回最近的整数格），确保起始图块能绝对、完美地覆盖视口的左侧外沿，不漏一像素黑边。
        int repeatIndex = (int)floor((viewLeft - baseLeftX) / repeatDrawW);
        
        // 3. 计算排在最左侧的第一个背景瓦片的“左边缘坐标”
        double currentLeftX = baseLeftX + repeatIndex * repeatDrawW;

        // 4. 从最左侧的起始点开始，不断向右平移一个完整 Width 的距离，直到把右侧视口全部盖满
        while (currentLeftX < viewRight)
        {
            // 如果生成的图块数量超出了池子的预设大小，安全中断，防止越界崩溃
            if (activeRenderCount >= (int)renderPool.size())
            {
                break;
            }

            // 从预分配的池子里取出一个槽位进行在位覆写，避免频繁 new/delete 造成卡顿
            BackgroundObject& repeatedObject = renderPool[activeRenderCount++];
            repeatedObject = object;

            // 标记这块背景是由平铺算法动态生成出来的克隆瓦片
            repeatedObject.generatedByTiling = true;

            // 将当前瓦片的“左边缘”加回半宽，转换为 Sprite 渲染所需的“中心点坐标”
            double currentCenterX = currentLeftX + repeatDrawW / 2.0;
            repeatedObject.runtimeCenterX = currentCenterX;
            repeatedObject.runtimeCenterY = renderCenterY;

            // 同步更新精灵的世界绘制数据
            repeatedObject.renderSprite.setWorldDrawData(
                currentCenterX,
                renderCenterY,
                repeatDrawW,
                repeatDrawH
            );

            // 往右累加一个背景图宽度，准备放置下一张
            currentLeftX += repeatDrawW;
        }
    }

    // 5. 将这一帧收集好的所有背景切片按照渲染深度 (renderOrder) 从小到大排序
    //    保证远景先画、近景后画，遮挡关系正确
    sort(
        renderPool.begin(),
        renderPool.begin() + activeRenderCount,
        [](const BackgroundObject& a, const BackgroundObject& b)
        {
            return a.renderOrder < b.renderOrder;
        }
    );
}

// 功能：清除当前的背景模板队列 and 本帧渲染缓冲。
void BackgroundManager::clear()
{
    objects.clear();
    activeRenderCount = 0;
}

// 功能：辅助方法，直接基于图片资源、渲染顺序、视差缩放系数和自动飘动速度等参数，
//       快捷创建一个背景对象并将其登记到背景队列中。
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
    double newDrawH,
    double newAutoScrollSpeedX
)
{
    // 若图片资源为空，安全拦截
    if (imageResource == NULL)
    {
        return;
    }

    BackgroundObject object;

    // 1. 设置渲染深度、视差因子、变焦响应因子与混合绘制模式
    object.setRenderData(
        newRenderOrder,
        newParallaxFactor,
        newZoomFactor,
        newUseAlphaBlend,
        newDrawMode
    );

    // 2. 设置其初始位置与尺寸
    object.setDrawData(
        newCenterX,
        newCenterY,
        newDrawW,
        newDrawH
    );

    // 3. 设定自主移动速度（如云层风力漂移速度，正数向右，负数向左）
    object.autoScrollSpeedX = newAutoScrollSpeedX;

    // 4. 绑定纹理并完成初次变换更新
    object.bindSpriteSource(imageResource);
    object.updateRuntimeTransform(0.0, 0.0);
    object.updateSprite();

    // 5. 将该图层模板登记到列表中
    objects.push_back(object);
    
    // 按渲染深度排好序，并立刻重建一次绘制列表
    sortObjectsByRenderOrder();
    rebuildRenderObjects();
}

// 功能：每帧调用。先驱动所有的背景模板更新它们自身的自主漂移和受相机速度带动的视差位移，
//       然后再调用平铺算法更新最终的绘制列表。
void BackgroundManager::updateRuntimeTransforms(double cameraVx, double cameraVy)
{
    // 遍历每一个注册 of 背景层模板，更新其坐标并同步到 Sprite 数据
    for (int i = 0; i < (int)objects.size(); i++)
    {
        objects[i].updateRuntimeTransform(cameraVx, cameraVy);
        objects[i].updateSprite();
    }

    // 更新完所有模板后，根据相机新视口重新进行裁剪平铺重建
    rebuildRenderObjects();
}

// 功能：按照渲染深度 (renderOrder) 由小到大排序模板背景，数值越小的图层在渲染时越先画。
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

// 功能：彻底清空所有的背景对象模板。
void BackgroundManager::clearObjects()
{
    objects.clear();
}

// 功能：向管理器手动添加一个已经构建好的背景对象模板。
void BackgroundManager::addObject(const BackgroundObject& object)
{
    objects.push_back(object);
    sortObjectsByRenderOrder();
}

// 功能：获取当前注册的背景模板图层数量。
int BackgroundManager::getObjectCount() const
{
    return (int)objects.size();
}

// 功能：获取当前注册的背景模板可写队列引用。
vector<BackgroundObject>& BackgroundManager::getObjects()
{
    return objects;
}

// 功能：获取当前注册的背景模板只读队列引用。
const vector<BackgroundObject>& BackgroundManager::getObjects() const
{
    return objects;
}
