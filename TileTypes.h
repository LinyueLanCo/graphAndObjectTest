#pragma once

// TileId:
// 地图 tile 编号。当前 TILE_EMPTY=0 表示空格，不绘制，也不生成默认地图碰撞。
enum TileId
{
    TILE_EMPTY = 0
};

// TileCollisionType:
// 定义地图碰撞层中每个格子的碰撞规则类型。
enum TileCollisionType
{
    TILE_COLLISION_NONE = 0,
    TILE_COLLISION_FULL_SOLID = 1,
    TILE_COLLISION_FULL_ONE_WAY = 2,
    TILE_COLLISION_TOP_HALF_ONE_WAY = 3,
    TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY = 4,
    TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY = 5
};

// TileInstance:
// 地图中一个实际摆放出来的 tile 实例。
struct TileInstance
{
    int tileId;

    int row;
    int col;

    // tile 实例的世界中心点。row/col 只负责生成默认位置，真正绘制以中心点为锚点。
    double centerX;
    double centerY;

    // tile 实例的绘制偏移，单位是世界坐标。
    double offsetX;
    double offsetY;

    // tile 实例的绘制缩放，默认 1 表示使用 TileMap 的默认绘制大小。
    double scaleX;
    double scaleY;

    int layer;

    int zIndex;

    bool visible;

    TileCollisionType collisionType;

    TileInstance();
};
