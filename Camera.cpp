#include "Camera.h"

Camera gCamera;

int worldToScreenX(double worldX)
{
	return gCamera.worldToScreenX(worldX);
}

int worldToScreenY(double worldY)
{
    return gCamera.worldToScreenY(worldY);
}

int worldSizeToScreen(double worldSize)
{
    return gCamera.worldSizeToScreen(worldSize);
}