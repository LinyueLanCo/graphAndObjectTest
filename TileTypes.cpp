#include "TileTypes.h"

// 功能：初始化一个空 tile 实例。
TileInstance::TileInstance()
{
    tileId = TILE_EMPTY;

    row = 0;
    col = 0;

    centerX = 0;
    centerY = 0;

    offsetX = 0;
    offsetY = 0;

    scaleX = 1.0;
    scaleY = 1.0;

    layer = 0;
    zIndex = 0;

    visible = true;

    collisionType = TILE_COLLISION_NONE;
}
