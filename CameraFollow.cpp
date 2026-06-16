#include "CameraFollow.h"

#include "Camera.h"

int gCameraFollowTargetIndex = 0;

void setCameraFollowTarget(int newTargetIndex, vector<Entity>& entitys)
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

    gCameraFollowTargetIndex = newTargetIndex;

    cout << "Camera follow target changed to Entity "
        << gCameraFollowTargetIndex << endl;
}

void updateCameraFollow(
    vector<Entity>& entitys,
    int worldWidth,
    int worldHeight,
    int mouseOffsetX,
    int mouseOffsetY
)
{
    int entityCount = (int)entitys.size();

    if (entityCount <= 0)
    {
        return;
    }

    if (gCameraFollowTargetIndex < 0 || gCameraFollowTargetIndex >= entityCount)
    {
        gCameraFollowTargetIndex = 0;
    }

    if (!entitys[gCameraFollowTargetIndex].getIsAlive())
    {
        gCameraFollowTargetIndex = 0;
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
        entitys[gCameraFollowTargetIndex].getX(),
        entitys[gCameraFollowTargetIndex].getY(),
        worldWidth,
        worldHeight,
        offsetWorldX,
        offsetWorldY
    );
}
