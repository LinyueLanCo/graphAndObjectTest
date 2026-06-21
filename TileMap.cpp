#include "TileMap.h"
// 引入 std::stringstream（字符串输入输出流模板类）。
// 为什么用它？因为在解析地图文件文本数据（rows, cols, 以及具体的二维瓦片网格 ID）时，
// 将地图文本字符串封装进 std::stringstream 中，能让我们像操作 std::ifstream 文件流一样，
// 直接使用流提取操作符 `>>` 来逐个提取整型和字符串，极大地方便了文本格式的解析工作。
#include <sstream>
#include "RenderQueue.h"

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
            instance.collisionType = getDefaultCollisionTypeByTileId(tileId);
            tileInstances.push_back(instance);
        }
    }
}

RectBox TileMap::getTileInstanceCollisionWorldBox(const TileInstance& tile) const
{
    double worldDrawW = drawTileWidth * tile.scaleX;
    double worldDrawH = drawTileHeight * tile.scaleY;

    double worldCenterX = tile.centerX + tile.offsetX;
    double worldCenterY = tile.centerY + tile.offsetY;

    RectBox box;
    box.left = worldCenterX - worldDrawW / 2.0;
    box.right = worldCenterX + worldDrawW / 2.0;
    box.bottom = worldCenterY - worldDrawH / 2.0;
    box.top = worldCenterY + worldDrawH / 2.0;

    TileCollisionType collisionType = tile.collisionType;

    if (collisionType == TILE_COLLISION_TOP_HALF_ONE_WAY)
    {
        box.bottom = box.top - worldDrawH * 0.5;
    }
    else if (collisionType == TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY)
    {
        box.right = box.left + worldDrawW * 0.5;
        box.bottom = box.top - worldDrawH * 0.5;
    }
    else if (collisionType == TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY)
    {
        box.left = box.left + worldDrawW * 0.5;
        box.bottom = box.top - worldDrawH * 0.5;
    }

    return box;
}

const TileInstance* TileMap::getTileInstanceUnder(const RectBox& entityBox, double checkDistance) const
{
    const TileInstance* result = nullptr;
    double highestY = -999999.0;

    for (const TileInstance& tile : tileInstances)
    {
        if (!tile.visible || tile.tileId == TILE_EMPTY || tile.collisionType == TILE_COLLISION_NONE)
        {
            continue;
        }

        RectBox tileBox = getTileInstanceCollisionWorldBox(tile);

        if (entityBox.right <= tileBox.left + EPS || entityBox.left >= tileBox.right - EPS)
        {
            continue;
        }

        if (entityBox.bottom >= tileBox.top - EPS && entityBox.bottom <= tileBox.top + checkDistance + EPS)
        {
            if (tileBox.top > highestY)
            {
                highestY = tileBox.top;
                result = &tile;
            }
        }
    }

    return result;
}

// 功能：从文本文件读取地图行列数和 tile id 数据。
bool TileMap::loadFromText(const std::string& mapContent)
{
    // 实例化 std::stringstream 模板流对象，将整个 mapContent 文本封入流中，
    // 以便后续可以直接利用 `>>` 操作符按照空格/换行符过滤并解析出整数。
    std::stringstream inFile(mapContent);

    release();

    // 从流中以对空白符敏感的格式读取地图的总行数和总列数
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
            // 通过 std::stringstream 流提取操作符，将字符形式的数字转换并赋值给二维整型网格
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

void TileMap::collectSprites(RenderQueue& queue)
{
    for (const TileInstance& tile : tileInstances)
    {
        if (!tile.visible || tile.tileId == TILE_EMPTY)
        {
            continue;
        }

        sprite tileSprite = buildSpriteFromTileInstance(tile);
        tileSprite.zIndex = tile.zIndex;
        queue.submit(tileSprite, SPRITE_TYPE_TILE, RGB(255, 220, 0));
    }
}
