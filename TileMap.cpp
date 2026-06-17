#include "TileMap.h"

// 功能：初始化 tile map 的尺寸、偏移和地图数据指针。
TileMap::TileMap()
{
    tileset = nullptr;
    rows = 0;
    cols = 0;

    sourceTileWidth = 16;
    sourceTileHeight = 16;

    drawTileWidth = 48;
    drawTileHeight = 48;

    tiles = NULL;
    collisionTiles = NULL;

    offsetX = 0;
    offsetY = 0;
}

// 功能：释放 tile map 动态分配的地图数据。
TileMap::~TileMap()
{
    release();
}

int TileMap::getRows()
{
    return rows;
}

int TileMap::getCols()
{
    return cols;
}

// 功能：获取当前地图中的 tile 实例数量。
int TileMap::getTileInstanceCount() const
{
    return (int)tileInstances.size();
}

// 功能：获取指定下标的 tile 实例。
const TileInstance& TileMap::getTileInstance(int index) const
{
    return tileInstances[index];
}

// 功能：获取当前地图的 tile 实例列表。
const vector<TileInstance>& TileMap::getTileInstances() const
{
    return tileInstances;
}

// 功能：设置指定 tile 实例的绘制偏移。
void TileMap::setTileInstanceOffset(int index, double newOffsetX, double newOffsetY)
{
    if (index < 0 || index >= (int)tileInstances.size())
    {
        return;
    }

    tileInstances[index].offsetX = newOffsetX;
    tileInstances[index].offsetY = newOffsetY;
}

// 功能：设置指定 tile 实例的绘制缩放。
void TileMap::setTileInstanceScale(int index, double newScaleX, double newScaleY)
{
    if (index < 0 || index >= (int)tileInstances.size())
    {
        return;
    }

    if (newScaleX <= 0 || newScaleY <= 0)
    {
        return;
    }

    tileInstances[index].scaleX = newScaleX;
    tileInstances[index].scaleY = newScaleY;
}

// 功能：同时设置指定 tile 实例的绘制偏移和缩放。
void TileMap::setTileInstanceTransform(
    int index,
    double newOffsetX,
    double newOffsetY,
    double newScaleX,
    double newScaleY
)
{
    setTileInstanceOffset(index, newOffsetX, newOffsetY);
    setTileInstanceScale(index, newScaleX, newScaleY);
}

// 功能：根据地图行列号查找对应的 tile 实例下标。
int TileMap::findTileInstanceIndexByGrid(int targetRow, int targetCol) const
{
    for (int i = 0; i < (int)tileInstances.size(); i++)
    {
        if (
            tileInstances[i].row == targetRow &&
            tileInstances[i].col == targetCol
            )
        {
            return i;
        }
    }

    return -1;
}

// 功能：根据地图行列号设置 tile 实例的绘制偏移和缩放。
void TileMap::setTileInstanceTransformByGrid(
    int targetRow,
    int targetCol,
    double newOffsetX,
    double newOffsetY,
    double newScaleX,
    double newScaleY
)
{
    int index = findTileInstanceIndexByGrid(targetRow, targetCol);

    if (index < 0)
    {
        return;
    }

    setTileInstanceTransform(
        index,
        newOffsetX,
        newOffsetY,
        newScaleX,
        newScaleY
    );
}

// 功能：把一个 TileInstance 转换为通用 sprite 数据，供 Renderer 绘制使用。
sprite TileMap::buildSpriteFromTileInstance(const TileInstance& tile)
{
    sprite tileSprite;

    tileSprite.visible = tile.visible;

    if (!tile.visible)
    {
        return tileSprite;
    }

    if (tile.tileId == TILE_EMPTY)
    {
        tileSprite.visible = false;
        return tileSprite;
    }

    if (sourceTileWidth <= 0 || sourceTileHeight <= 0)
    {
        tileSprite.visible = false;
        return tileSprite;
    }

    if (tileset == nullptr)
    {
        tileSprite.visible = false;
        return tileSprite;
    }

    int tilesetCols = tileset->getWidth() / sourceTileWidth;

    if (tilesetCols <= 0)
    {
        tileSprite.visible = false;
        return tileSprite;
    }

    int realTileIndex = tile.tileId - 1;

    // 用 tileId 在 tileset 中的线性序号换算出原图的裁剪坐标。
    int srcX = (realTileIndex % tilesetCols) * sourceTileWidth;
    int srcY = (realTileIndex / tilesetCols) * sourceTileHeight;

    tileSprite.setSource(
        tileset,
        srcX,
        srcY,
        sourceTileWidth,
        sourceTileHeight
    );

    // 用默认 tile 世界尺寸乘以实例缩放，得到本帧最终绘制尺寸。
    double worldDrawW = drawTileWidth * tile.scaleX;
    double worldDrawH = drawTileHeight * tile.scaleY;

    double worldCenterX = tile.centerX + tile.offsetX;
    double worldCenterY = tile.centerY + tile.offsetY;

    tileSprite.setWorldDrawData(
        worldCenterX,
        worldCenterY,
        worldDrawW,
        worldDrawH
    );

    return tileSprite;
}

// 功能：释放并清空当前地图碰撞层二维数组。
void TileMap::releaseCollisionTiles()
{
    if (collisionTiles != NULL)
    {
        for (int row = 0; row < rows; row++)
        {
            delete[] collisionTiles[row];
        }

        delete[] collisionTiles;
        collisionTiles = NULL;
    }
}

// 功能：释放并清空当前地图二维数组。
void TileMap::release()
{
    if (tiles != NULL)
    {
        for (int row = 0; row < rows; row++)
        {
            delete[] tiles[row];
        }

        delete[] tiles;
        tiles = NULL;
    }

    releaseCollisionTiles();

    rows = 0;
    cols = 0;
}

// 功能：设置 tileset 原始 tile 尺寸和世界绘制 tile 尺寸。
void TileMap::setTileSize(
    int newSourceTileWidth,
    int newSourceTileHeight,
    int newDrawTileWidth,
    int newDrawTileHeight
)
{
    sourceTileWidth = newSourceTileWidth;
    sourceTileHeight = newSourceTileHeight;

    drawTileWidth = newDrawTileWidth;
    drawTileHeight = newDrawTileHeight;
}

// 功能：设置整张 tile map 在世界坐标中的偏移。
void TileMap::setOffset(double newOffsetX, double newOffsetY)
{
    offsetX = newOffsetX;
    offsetY = newOffsetY;
}

// 功能：加载 tile map 使用的 tileset 图片。
void TileMap::loadTileset(Image2D* tilesetImage)
{
    tileset = tilesetImage;
}

// 功能：根据视觉 tile id 返回默认碰撞类型。
TileCollisionType TileMap::getDefaultCollisionTypeByTileId(int tileId)
{
    if (tileId == TILE_EMPTY)
    {
        return TILE_COLLISION_NONE;
    }

    switch (tileId)
    {
    case 13:
    case 14:
    case 15:
    case 16:
    case 18:
    case 19:
    case 20:
    case 21:
    case 35:
    case 36:
    case 37:
    case 38:
    case 40:
    case 41:
    case 42:
    case 58:
    case 59:
    case 60:
    case 62:
    case 63:
    case 64:
    case 101:
    case 102:
    case 103:
    case 104:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 123:
    case 124:
    case 125:
    case 126:
    case 128:
    case 129:
    case 130:
    case 131:
    case 132:
    case 146:
    case 147:
    case 148:
    case 150:
    case 151:
    case 152:
    case 189:
    case 190:
    case 191:
    case 192:
    case 194:
    case 195:
    case 196:
    case 197:
    case 211:
    case 212:
    case 213:
    case 214:
    case 216:
    case 217:
    case 218:
    case 219:
    case 234:
    case 235:
    case 236:
    case 238:
    case 239:
    case 240:
    case 241:
        return TILE_COLLISION_FULL_SOLID;
    }

    if (tileId == 33 || tileId == 55)
    {
        return TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY;
    }

    if (
        tileId == 7 ||
        tileId == 8 ||
        tileId == 9 ||
        tileId == 73 ||
        tileId == 74 ||
        tileId == 75 ||
        tileId == 76 ||
        tileId == 77
        )
    {
        return TILE_COLLISION_TOP_HALF_ONE_WAY;
    }

    if (tileId == 32 || tileId == 54)
    {
        return TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY;
    }

    return TILE_COLLISION_NONE;
}

// 功能：根据视觉 tile 数据自动生成默认碰撞层。
void TileMap::generateDefaultCollisionFromTiles()
{
    releaseCollisionTiles();

    collisionTiles = new int* [rows];

    for (int row = 0; row < rows; row++)
    {
        collisionTiles[row] = new int[cols];

        for (int col = 0; col < cols; col++)
        {
            int tileId = tiles[row][col];

            collisionTiles[row][col] = getDefaultCollisionTypeByTileId(tileId);
        }
    }
}

// 功能：根据当前 tiles 二维数组重建 tile 实例列表。
void TileMap::rebuildTileInstances()
{
    tileInstances.clear();

    if (tiles == NULL)
    {
        return;
    }

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            int tileId = tiles[row][col];

            if (tileId == TILE_EMPTY)
            {
                continue;
            }

            TileInstance instance;

            instance.tileId = tileId;
            instance.row = row;
            instance.col = col;

            // 用地图偏移、列号和反向行号，计算这个 tile 默认所在网格的世界左下角。
            double gridWorldLeft = offsetX + col * drawTileWidth;
            double gridWorldBottom = offsetY + (rows - 1 - row) * drawTileHeight;

            // 用网格左下角加半个单元格，得到 tile 实例的默认世界中心点。
            instance.centerX = gridWorldLeft + drawTileWidth / 2.0;
            instance.centerY = gridWorldBottom + drawTileHeight / 2.0;

            instance.offsetX = 0;
            instance.offsetY = 0;

            instance.scaleX = 1.0;
            instance.scaleY = 1.0;

            instance.layer = 0;
            instance.visible = true;
            tileInstances.push_back(instance);
        }
    }
}

// 功能：从文本文件读取地图行列数和 tile id 数据。
bool TileMap::loadFromText(const std::string& mapContent)
{
    std::stringstream inFile(mapContent);

    release();

    inFile >> rows >> cols;

    if (rows <= 0 || cols <= 0)
    {
        cout << "Invalid map size." << endl;
        return false;
    }

    tiles = new int* [rows];

    for (int row = 0; row < rows; row++)
    {
        tiles[row] = new int[cols];

        for (int col = 0; col < cols; col++)
        {
            inFile >> tiles[row][col];
        }
    }

    generateDefaultCollisionFromTiles();
    rebuildTileInstances();

    return true;
}

// 功能：获取指定行列的 tile 碰撞类型。
TileCollisionType TileMap::getTileCollisionType(int row, int col)
{
    if (row < 0 || row >= rows || col < 0 || col >= cols)
    {
        return TILE_COLLISION_NONE;
    }

    if (collisionTiles == NULL)
    {
        return TILE_COLLISION_NONE;
    }

    return (TileCollisionType)collisionTiles[row][col];
}

// 功能：判断指定 tile 是否拥有任何地图碰撞类型。
bool TileMap::hasTileCollision(int row, int col)
{
    return getTileCollisionType(row, col) != TILE_COLLISION_NONE;
}

// 功能：获取指定 tile 在世界坐标中的矩形范围。
RectBox TileMap::getTileWorldBox(int row, int col)
{
    RectBox box;

    // 用地图偏移、列号和反向行号，计算 tile 的世界左下角。
    double worldLeft = offsetX + col * drawTileWidth;
    double worldBottom = offsetY + (rows - 1 - row) * drawTileHeight;

    // 用左下角加绘制宽高，得到 tile 的世界 AABB。
    box.left = worldLeft;
    box.right = worldLeft + drawTileWidth;
    box.bottom = worldBottom;
    box.top = worldBottom + drawTileHeight;

    return box;
}

// 功能：根据 tile 碰撞类型，获取指定 tile 实际参与碰撞的世界矩形范围。
RectBox TileMap::getTileCollisionWorldBox(int row, int col)
{
    RectBox tileBox = getTileWorldBox(row, col);
    RectBox collisionBox = tileBox;

    TileCollisionType collisionType = getTileCollisionType(row, col);

    double tileWidth = tileBox.right - tileBox.left;
    double tileHeight = tileBox.top - tileBox.bottom;

    if (collisionType == TILE_COLLISION_TOP_HALF_ONE_WAY)
    {
        // 由完整 tile 顶边向下取半格高度，得到平台主体的实际碰撞盒。
        collisionBox.bottom = tileBox.top - tileHeight * 0.5;
        collisionBox.top = tileBox.top;
    }
    else if (collisionType == TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY)
    {
        // 由完整 tile 左边向右取半格宽度，并由顶边向下取半格高度。
        collisionBox.right = tileBox.left + tileWidth * 0.5;
        collisionBox.bottom = tileBox.top - tileHeight * 0.5;
        collisionBox.top = tileBox.top;
    }
    else if (collisionType == TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY)
    {
        // 由完整 tile 中线向右取半格宽度，并由顶边向下取半格高度。
        collisionBox.left = tileBox.left + tileWidth * 0.5;
        collisionBox.bottom = tileBox.top - tileHeight * 0.5;
        collisionBox.top = tileBox.top;
    }

    return collisionBox;
}

// 功能：绘制所有拥有地图碰撞类型的 tile 调试碰撞框。
void TileMap::drawDebugCollisionBoxes()
{
    setlinecolor(YELLOW);

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            if (!hasTileCollision(row, col))
            {
                continue;
            }

            TileCollisionType collisionType = getTileCollisionType(row, col);

            if (collisionType == TILE_COLLISION_FULL_SOLID)
            {
                setlinecolor(YELLOW);
            }
            else
            {
                setlinecolor(GREEN);
            }

            RectBox box = getTileCollisionWorldBox(row, col);

            int screenLeft = worldToScreenX(box.left);
            int screenRight = worldToScreenX(box.right);
            int screenTop = worldToScreenY(box.top);
            int screenBottom = worldToScreenY(box.bottom);

            rectangle(screenLeft, screenTop, screenRight, screenBottom);
        }
    }
}

// 功能：获取当前 tile map 对应的世界宽度。
int TileMap::getworldWidth()
{
    return cols * drawTileWidth * 3;
}

// 功能：获取当前 tile map 对应的世界高度。
int TileMap::getWOrldHeight()
{
    return rows * drawTileHeight * 3;
}
