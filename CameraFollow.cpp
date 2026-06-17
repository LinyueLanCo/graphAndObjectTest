#include "CameraFollow.h"
#include <iostream>
#include "Camera.h"

std::string gCameraFollowTargetId = "Player1";

void setCameraFollowTarget(const std::string& newTargetId, const EntityManager& entityManager)
{
    const Entity* target = entityManager.getEntityById(newTargetId);
    if (!target)
    {
        return;
    }

    if (!target->getIsAlive())
    {
        return;
    }

    gCameraFollowTargetId = newTargetId;

    std::cout << "Camera follow target changed to Entity "
        << gCameraFollowTargetId << std::endl;
}

void updateCameraFollow(
    EntityManager& entityManager,
    int worldWidth,
    int worldHeight,
    int mouseOffsetX,
    int mouseOffsetY
)
{
    const std::vector<Entity>& entities = entityManager.getEntities();
    if (entities.empty())
    {
        return;
    }

    Entity* target = entityManager.getEntityById(gCameraFollowTargetId);
    if (!target || !target->getIsAlive())
    {
        // 尝试回退到默认的 Player1
        target = entityManager.getEntityById("Player1");
        if (target && target->getIsAlive())
        {
            gCameraFollowTargetId = "Player1";
        }
        else
        {
            // 回退到第一个存活的实体
            for (auto& e : entityManager.getEntities())
            {
                if (e.getIsAlive())
                {
                    target = &e;
                    gCameraFollowTargetId = e.getId();
                    break;
                }
            }
        }
    }

    if (!target)
    {
        return;
    }

    if (GetAsyncKeyState('B') & 0x8000)
    {
        gCamera.zoomTo(0.3);
    }
    else if (GetAsyncKeyState('V') & 0x8000)
    {
        gCamera.zoomTo(3.0);
    }
    else
    {
        gCamera.zoomTo(1.0);
    }

    // 先更新 zoom，再用新的可见视口范围限制 camera center。
    gCamera.updateZoom();

    // 鼠标引导相机偏移强度：0.25 表示鼠标偏移的 25% 用于相机偏移。
    double lookStrength = 0.25;

    // 死区用于过滤鼠标在中心附近的轻微抖动。
    int deadZone = 20;

    if (abs(mouseOffsetX) < deadZone)
    {
        mouseOffsetX = 0;
    }

    if (abs(mouseOffsetY) < deadZone)
    {
        mouseOffsetY = 0;
    }

    // 屏幕像素偏移除以 zoom，得到对应的世界坐标偏移。
    double offsetWorldX = mouseOffsetX / gCamera.zoom * lookStrength;

    // 屏幕 Y 轴向下为正，世界 Y 轴向上为正，所以这里用负号翻转方向。
    double offsetWorldY = -mouseOffsetY / gCamera.zoom * lookStrength;

    gCamera.followSmooth(
        target->getX(),
        target->getY(),
        worldWidth,
        worldHeight,
        offsetWorldX,
        offsetWorldY
    );
}
