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

    int** tiles;
    int** collisionTiles;

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
    void drawDebugCollisionBoxes();

    int getworldWidth();
    int getWOrldHeight();
};
