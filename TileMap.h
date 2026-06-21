#pragma once

#include "Camera.h"
#include "Collision.h"
#include "Config.h"
#include "Resource.h"
#include "Sprite.h"
#include "TileTypes.h"

// TileMap:
// 负责加载 tileset 和地图文本数据，并维护地图中的 tile 实例与碰撞层数据。
class TileMap
{
private:
    Image2D* tileset;

    int rows;
    int cols;

    int sourceTileWidth;
    int sourceTileHeight;

    int drawTileWidth;
    int drawTileHeight;

    // tiles 和 collisionTiles:
    // int** 代表指向指针的二级指针，在 C++ 中常用于实现二维动态分配数组。
    // 这里用于保存地图网格和碰撞网格的行列矩阵，需要在运行时动态分配并手动进行析构释放以防止内存泄漏。
    int** tiles;
    int** collisionTiles;

    // tileInstances: 地图上被实例化生成的图块列表。
    // std::vector 容器用于在连续内存中存储所有活动的图块实例，方便高效顺序渲染。
    vector<TileInstance> tileInstances;

    double offsetX;
    double offsetY;

public:
    TileMap();
    ~TileMap();

    int getRows();
    int getCols();

    int getTileInstanceCount() const;
    const TileInstance& getTileInstance(int index) const;
    const vector<TileInstance>& getTileInstances() const;

    void setTileInstanceOffset(int index, double newOffsetX, double newOffsetY);
    void setTileInstanceScale(int index, double newScaleX, double newScaleY);
    void setTileInstanceTransform(
        int index,
        double newOffsetX,
        double newOffsetY,
        double newScaleX,
        double newScaleY
    );
    int findTileInstanceIndexByGrid(int targetRow, int targetCol) const;
    void setTileInstanceTransformByGrid(
        int targetRow,
        int targetCol,
        double newOffsetX,
        double newOffsetY,
        double newScaleX,
        double newScaleY
    );

    sprite buildSpriteFromTileInstance(const TileInstance& tile);

    void releaseCollisionTiles();
    void release();

    void setTileSize(
        int newSourceTileWidth,
        int newSourceTileHeight,
        int newDrawTileWidth,
        int newDrawTileHeight
    );
    void setOffset(double newOffsetX, double newOffsetY);
    void loadTileset(Image2D* tilesetImage);

    TileCollisionType getDefaultCollisionTypeByTileId(int tileId);
    void generateDefaultCollisionFromTiles();
    void rebuildTileInstances();
    RectBox getTileInstanceCollisionWorldBox(const TileInstance& tile) const;
    const TileInstance* getTileInstanceUnder(const RectBox& entityBox, double checkDistance = 2.0) const;
    bool loadFromText(const std::string& mapContent);

    TileCollisionType getTileCollisionType(int row, int col);
    bool hasTileCollision(int row, int col);
    RectBox getTileWorldBox(int row, int col);
    RectBox getTileCollisionWorldBox(int row, int col);
    void collectSprites(class RenderQueue& queue);

    int getworldWidth();
    int getWOrldHeight();
};
