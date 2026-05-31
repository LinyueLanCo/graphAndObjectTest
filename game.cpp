#include <graphics.h>
#include <windows.h>
#include <conio.h>
#include <cmath>
#include <iostream>
#include<fstream>
//后续会添加外部
using namespace std;

#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib,"winmm.lib")
const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 900;
const int ENTITY_COUNT = 7;

const double EPS = 0.001;

// 重力相关参数
const double GRAVITY = 1.98;
const double JUMP_SPEED = 28.0;
const double MAX_FALL_SPEED = -28.0;

/*
============================================================
当前代码的核心数据流向
============================================================

main()
    -> 创建 InputManager 和 Level
    -> level.init() 负责加载当前关卡的地图、背景、实体、UI、初始状态
    -> while 主循环：
        1. input.update()
           统一采集键盘/鼠标状态，形成 keyNow/keyLast 等输入缓存。

        2. level.update(input)
           Level 作为当前关卡的调度者，决定各系统的更新顺序：
           清理上一帧临时状态 -> 生成玩家意图 -> 移动系统 -> 碰撞系统
           -> 相机/UI输入 -> 调试状态输出 -> 重叠事件 -> UI更新。

        3. cleardevice()
        4. level.draw()
           绘制背景、地图、实体、UI。

        5. FlushBatchDraw()
           将本帧绘制结果显示出来。

玩家移动数据流：
    键盘输入
        -> InputManager
        -> PlayerController::makeIntent()
        -> BehaviorIntent
        -> MovementHandle::update()
        -> CollisionHandle 计算 allowedMoveX / allowedMoveY
        -> MovementHandle 把允许位移写回 Entity 的 x/y/velocity/onGround 等状态
        -> Entity::updateAnimationByIntent()
        -> Entity::draw()

职责边界：
    InputManager      只负责“输入采集”
    PlayerController  只负责“输入 -> 行为意图”
    BehaviorIntent    只保存“这一帧想做什么”
    MovementHandle    负责“速度、冲刺、跳跃、重力、期望位移”
    CollisionHandle   负责“阻挡检测、允许位移、世界边界修正、重叠判断”
    Level             负责“当前关卡的对象持有、初始化、更新调度和绘制调度”
    Entity            主要保存实体数据，并提供碰撞盒、动画、绘制等接口
============================================================
*/


// 坐标转换
// 当前工程使用“世界坐标”和“屏幕坐标”两套坐标：
// - 世界坐标：逻辑坐标，原点在左下角，Entity 的 x/y 表示实体中心点。
// - 屏幕坐标：EasyX 绘制坐标，原点在窗口左上角。
// - Camera 记录当前视口在世界坐标中的左下角位置和缩放系数 zoom。
// 公式：
//   screenX = (worldX - cameraX) * zoom
//   screenY = WINDOW_HEIGHT - (worldY - cameraY) * zoom
//   screenSize = worldSize * zoom


//做临时用，枚举实体类型

enum EntityType
{
    PLAYER = 1,
    ENTITY = 2,
    COIN = 3,
    DEFAULT = 4
};



// Camera：
 // 负责把“世界中的一个区域”映射到屏幕上。
 // x/y 表示摄像机左下角在世界坐标中的位置；zoom 表示缩放。
 // followSmooth 使用 lerp 公式做平滑跟随：
 //   current += (target - current) * speed
 // 这个公式的效果是：距离目标越远移动越快，越接近目标越慢。
struct Camera
{
    double x;
    double y;

    double zoom;

    double targetX;
    double targetY;
    double targetZoom;

    // 功能：初始化相机位置、目标位置和缩放参数。
    Camera()
    {
        x = 0;
        y = 0;

        zoom = 1.0;

        targetX = 0;
        targetY = 0;
        targetZoom = 1.0;
    }

    // 功能：计算当前相机缩放下屏幕可见的世界宽度。
    double getVisibleWorldWidth()
    {
        return WINDOW_WIDTH / zoom;
    }

    // 功能：计算当前相机缩放下屏幕可见的世界高度。
    double getVisibleWorldHeight()
    {
        return WINDOW_HEIGHT / zoom;
    }

    // 功能：让相机立即居中跟随目标点，并限制在世界范围内。
    void followInstant(double targetWorldX, double targetWorldY, int worldWidth, int worldHeight)
    {
        double visibleW = getVisibleWorldWidth();
        double visibleH = getVisibleWorldHeight();

        x = targetWorldX - visibleW / 2.0;
        y = targetWorldY - visibleH / 2.0;

        limitInWorld(worldWidth, worldHeight);
    }

    // 功能：让相机平滑跟随目标点，并叠加鼠标观察偏移。
    void followSmooth(
        double targetWorldX,
        double targetWorldY,
        int worldWidth,
        int worldHeight,
        double offsetWorldX,
        double offsetWorldY
    )
    {
        double visibleW = getVisibleWorldWidth();
        double visibleH = getVisibleWorldHeight();

        targetX = targetWorldX - visibleW / 2.0 + offsetWorldX;
        targetY = targetWorldY - visibleH / 2.0 + offsetWorldY;

        // 数值越小越“拖”，越大越紧跟
        double followSpeed = 0.08;

        x += (targetX - x) * followSpeed;
        y += (targetY - y) * followSpeed;

        limitInWorld(worldWidth, worldHeight);
    }

    // 功能：设置相机目标缩放值，并限制缩放范围。
    void zoomTo(double newZoom)
    {
        if (newZoom < 0.2)
        {
            newZoom = 0.2;
        }

        if (newZoom > 5.0)
        {
            newZoom = 5.0;
        }

        targetZoom = newZoom;
    }

    // 功能：平滑更新相机当前缩放，使其靠近目标缩放。
    void updateZoom()
    {
        double zoomSpeed = 0.08;
        zoom += (targetZoom - zoom) * zoomSpeed;
    }

    // 将摄像机视口限制在世界范围内。
    // 这里修正的是摄像机左下角 x/y，而不是实体坐标。
    // 如果世界尺寸小于可见视口，就让摄像机居中显示这个世界。
    // 功能：把相机可见范围限制在关卡世界边界内。
    void limitInWorld(int worldWidth, int worldHeight)
    {
        double visibleW = getVisibleWorldWidth();
        double visibleH = getVisibleWorldHeight();

        if (worldWidth <= visibleW)
        {
            x = (worldWidth - visibleW) / 2.0;
        }
        else
        {
            if (x < 0)
            {
                x = 0;
            }

            if (x > worldWidth - visibleW)
            {
                x = worldWidth - visibleW;
            }
        }

        if (worldHeight <= visibleH)
        {
            y = (worldHeight - visibleH) / 2.0;
        }
        else
        {
            if (y < 0)
            {
                y = 0;
            }

            if (y > worldHeight - visibleH)
            {
                y = worldHeight - visibleH;
            }
        }
    }

};
Camera gCamera;

// 功能：把世界坐标 X 转换为屏幕坐标 X。
int worldToScreenX(double worldX)
{
    return (int)((worldX - gCamera.x) * gCamera.zoom);
}

// 功能：把世界坐标 Y 转换为 EasyX 屏幕坐标 Y。
int worldToScreenY(double worldY)
{
    return (int)(WINDOW_HEIGHT - (worldY - gCamera.y) * gCamera.zoom);
}

// 功能：把世界空间尺寸转换为当前缩放下的屏幕尺寸。
int worldSizeToScreen(double worldSize)
{
    return (int)(worldSize * gCamera.zoom);
}

// Sound：
 // 音效系统占位类。当前项目暂时直接使用 PlaySoundW 播放音效，
 // 后续可以把音效播放、循环、停止、isPlaying 等状态统一封装进这里。
class Sound
{



};
// AnimationState：
 // 玩家动画状态枚举。当前主要用于 Entity::changeAnimation()
 // 根据移动意图和 sprinting 状态切换待机、行走、奔跑动画。
enum AnimationState
{
    ANIM_IDLE_L,
    ANIM_IDLE_R,
    ANIM_WALK_LEFT,
    ANIM_WALK_RIGHT,
    ANIM_RUN_LEFT,
    ANIM_RUN_RIGHT,
    ANIM_COUNT
};
// facingDirection：
 // 记录实体最后一次有效朝向。
 // 没有移动输入时，待机动画会根据这个方向决定使用左待机还是右待机。
enum facingDirection
{
    LEFT,
    RIGHT,
    UP,
    DOWN

};


// putimage_alpha：
 // 使用 AlphaBlend 绘制带透明通道的 IMAGE，解决普通 putimage 透明通道不正确的问题。
// 功能：按原图尺寸绘制带 Alpha 通道的图片。
inline void putimage_alpha(int x, int y, IMAGE* img)
{
    int w = img->getwidth();
    int h = img->getheight();

    BLENDFUNCTION blend;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    AlphaBlend(
        GetImageHDC(NULL),
        x, y, w, h,
        GetImageHDC(img),
        0, 0, w, h,
        blend
    );
}
// putimage_alpha 重载：
 // 支持指定绘制宽高，用于对整张透明图片进行简单缩放绘制。
// 功能：按指定尺寸缩放绘制带 Alpha 通道的图片。
inline void putimage_alpha(int x, int y, int drawW, int drawH, IMAGE* img)
{
    int sourceW = img->getwidth();
    int sourceH = img->getheight();

    if (drawW < 1)
    {
        drawW = 1;
    }

    if (drawH < 1)
    {
        drawH = 1;
    }

    BLENDFUNCTION blend;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    AlphaBlend(
        GetImageHDC(NULL),
        x, y, drawW, drawH,
        GetImageHDC(img),
        0, 0, sourceW, sourceH,
        blend
    );
}
// putimage_alpha_tile：
 // 从 tileset 或 sprite sheet 中截取一块源矩形，并绘制到目标屏幕矩形。
 // TileMap 和 animatedSprite 都依赖这个函数完成局部贴图绘制。
// 功能：从图集中裁剪指定区域并以 Alpha 混合绘制到屏幕。
inline void putimage_alpha_tile(
    int x,
    int y,
    int drawW,
    int drawH,
    IMAGE* img,
    int srcX,
    int srcY,
    int srcW,
    int srcH
)
{
    BLENDFUNCTION blend;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    AlphaBlend
    (
        GetImageHDC(NULL),
        x, y, drawW, drawH,
        GetImageHDC(img),
        srcX, srcY, srcW, srcH,
        blend
    );
}
// animatedSprite：
 // 带帧动画支持的精灵类。
 // 它只关心“图片帧如何播放”和“如何根据 Entity 的世界坐标绘制到屏幕上”。
 // 注意：它不负责实体逻辑、不负责输入、不负责碰撞。
 // 绘制公式：
 //   worldDrawW = frameWidth * scaleX
 //   worldDrawH = frameHeight * scaleY
 //   spriteCenter = ownerPosition + offset
 //   drawLeft = spriteCenterX - worldDrawW / 2
 //   drawTop  = spriteCenterY + worldDrawH / 2
 // 再通过 worldToScreenX/Y 转成屏幕坐标。
class animatedSprite
{
private:
    IMAGE image;
    int frameCount;
    int currentFrame;
    int frameWidth;
    int frameHeight;
    bool isPlaying;
    bool isLoop;

    int frameInterval;//帧间隔
    int frameTimer;//计时器
    double scaleX;
    double scaleY;
    double offsetX;
    double offsetY;
public:
    // 功能：初始化序列帧动画的默认播放参数。
    animatedSprite()//基础构造
    {
        frameWidth = 0;
        frameHeight = 0;
        frameCount = 0;
        currentFrame = 0;

        frameInterval = 8;
        frameTimer = 0;

        isPlaying = true;
        isLoop = true;

        scaleX = 1.0;
        scaleY = 1.0;
        offsetX = 0;
        offsetY = 0;
    }
    // 功能：按显式帧尺寸加载序列帧图片。
    void load(const TCHAR* path, int frameWidth, int frameHeight, int frameCount)
    {
        loadimage(&image, path);
        this->frameWidth = frameWidth;
        this->frameHeight = frameHeight;
        this->frameCount = frameCount;

        currentFrame = 0;
        frameTimer = 0;
        isPlaying = 1;

    }
    // 功能：按帧数自动平均切分横向序列帧图片。
    void load(const TCHAR* path, int newFrameCount)
    {
        loadimage(&image, path);

        frameCount = newFrameCount;

        if (frameCount < 1)
        {
            frameCount = 1;
        }

        frameWidth = image.getwidth() / frameCount;
        frameHeight = image.getheight();

        currentFrame = 0;
        frameTimer = 0;
        isPlaying = true;
    }
    // 功能：获取当前动画单帧宽度。
    int getFrameWidth()
    {
        return frameWidth;
    }

    // 功能：获取当前动画单帧高度。
    int getFrameHeight()
    {
        return frameHeight;
    }
    // 功能：设置动画帧切换间隔。
    void setSpeed(int frameInterval)//设置帧间隔切换速度，也就是动画播放速度
    {
        if (frameInterval < 1)
        {
            frameInterval = 1;
        }
        this->frameInterval = frameInterval;

    }
    // 功能：设置动画是否循环播放。
    void setLoop(bool value)//设置是否循环
    {
        isLoop = value;
    }
    // 功能：停止当前动画播放。
    void stop()//停止播放设置
    {
        isPlaying = false;
    }
    // 功能：设置动画绘制缩放和相对实体中心的偏移。
    void setTransform(double newScaleX, double newScaleY, double newOffsetX, double newOffsetY)
    {
        scaleX = newScaleX;
        scaleY = newScaleY;
        offsetX = newOffsetX;
        offsetY = newOffsetY;
    }
    // 功能：重置动画到第一帧并清空计时器。
    void reset()
    {
        currentFrame = 0;
        frameTimer = 0;

    }
    // 功能：推进动画帧计时并在需要时切换当前帧。
    void update()
    {
        if (!isPlaying)
        {
            return;
        }
        if (frameCount <= 0)
        {
            return;
        }
        frameTimer++;
        if (frameTimer >= frameInterval)
        {
            frameTimer = 0;//每自增到interval，重置计数器
            currentFrame++;
            if (currentFrame >= frameCount)
            {
                if (isLoop)//判定是否启动循环播放，为真则当当前帧超出帧总数时重置回第一帧
                {
                    currentFrame = 0;
                }
                else//不启用loop，则使用最后一帧画面做结束画面，停止播放
                {
                    currentFrame = frameCount - 1;
                    isPlaying = false;

                }
            }
        }

    }
    // 功能：根据拥有者世界坐标把当前动画帧绘制到屏幕。
    void draw(double ownerX, double ownerY)
    {
        if (frameCount <= 0)
        {
            return;
        }

        double worldDrawW = frameWidth * scaleX;
        double worldDrawH = frameHeight * scaleY;

        double spriteCenterX = ownerX + offsetX;
        double spriteCenterY = ownerY + offsetY;

        double worldLeft = spriteCenterX - worldDrawW / 2.0;
        double worldTop = spriteCenterY + worldDrawH / 2.0;

        int drawX = worldToScreenX(worldLeft);
        int drawY = worldToScreenY(worldTop);

        int screenDrawW = worldSizeToScreen(worldDrawW);
        int screenDrawH = worldSizeToScreen(worldDrawH);

        int srcX = currentFrame * frameWidth;
        int srcY = 0;

        putimage_alpha_tile(
            drawX,
            drawY,
            screenDrawW,
            screenDrawH,
            &image,
            srcX,
            srcY,
            frameWidth,
            frameHeight
        );
    }
};


// RectBox：
 // 世界坐标下真正用于检测的 AABB 矩形盒。
 // left/right/bottom/top 分别表示四条边。
 // 当前项目中的碰撞检测本质都是基于这个结构。
struct RectBox
{
    double left;
    double right;
    double bottom;
    double top;
};

// CollisionBox：
 // Entity 本地碰撞盒配置。
 // width/height 通常来自精灵原始帧尺寸。
 // scaleX/scaleY 用于把碰撞盒按实体缩放。
 // offsetX/offsetY 用于让碰撞盒中心相对实体中心产生偏移。
struct CollisionBox
{
    double width;
    double height;
    double offsetX;
    double offsetY;
    double scaleX;
    double scaleY;
};

// TileId：
 // 地图 tile 编号。当前 TILE_EMPTY=0 表示空格，不绘制也不视为 solid tile。
enum TileId
{
    TILE_EMPTY = 0
};

// TileMap：
 // 负责加载 tileset 和地图文本数据，并把 tile 绘制到世界坐标中。
 // 当前地图只负责显示，实体阻挡暂时主要来自 Entity 的 blocking 碰撞盒。
 // 后续可以扩展为：tile 碰撞、地图触发器、地图层级、视口剔除等。
class TileMap
{
private:
    IMAGE tileset;

    int rows;
    int cols;

    // 原始 tileset 中每个 tile 的大小，比如你的图是 16x16
    int sourceTileWidth;
    int sourceTileHeight;

    // 实际显示到游戏世界里的大小，比如放大到 48x48
    int drawTileWidth;
    int drawTileHeight;

    // 动态二维数组
    int** tiles;

    // 整张地图在世界坐标里的偏移
    double offsetX;
    double offsetY;

public:
    // 功能：初始化 tile map 的尺寸、偏移和地图数据指针。
    TileMap()
    {
        rows = 0;
        cols = 0;

        sourceTileWidth = 16;
        sourceTileHeight = 16;

        drawTileWidth = 48;
        drawTileHeight = 48;

        tiles = NULL;

        offsetX = 0;
        offsetY = 0;
    }

    // 功能：释放 tile map 动态分配的地图数据。
    ~TileMap()
    {
        release();
    }

    // 功能：释放并清空当前地图二维数组。
    void release()
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

        rows = 0;
        cols = 0;
    }

    // 功能：设置 tileset 原始 tile 尺寸和世界绘制 tile 尺寸。
    void setTileSize(
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
    void setOffset(double newOffsetX, double newOffsetY)
    {
        offsetX = newOffsetX;
        offsetY = newOffsetY;
    }

    // 功能：加载 tile map 使用的 tileset 图片。
    void loadTileset(const TCHAR* imagePath)
    {
        loadimage(&tileset, imagePath);
    }

    // 功能：从文本文件读取地图行列数和 tile id 数据。
    bool loadFromFile(const char* mapPath)
    {
        ifstream inFile(mapPath);

        if (!inFile.is_open())
        {
            cout << "Failed to open map file: " << mapPath << endl;
            return false;
        }

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

        inFile.close();

        return true;
    }

    // 功能：遍历并绘制整张 tile map。
    void draw()
    {
        for (int row = 0; row < rows; row++)
        {
            for (int col = 0; col < cols; col++)
            {
                drawTile(row, col);
            }
        }
    }

    // 功能：绘制指定行列的单个 tile。
    void drawTile(int row, int col)
    {
        int tileId = tiles[row][col];

        if (tileId == TILE_EMPTY)
        {
            return;
        }

        int realTileIndex = tileId - 1;

        int tilesetCols = tileset.getwidth() / sourceTileWidth;

        int srcX = (realTileIndex % tilesetCols) * sourceTileWidth;
        int srcY = (realTileIndex / tilesetCols) * sourceTileHeight;

        double worldLeft = offsetX + col * drawTileWidth;
        double worldBottom = offsetY + (rows - 1 - row) * drawTileHeight;
        double worldRight = worldLeft + drawTileWidth;
        double worldTop = worldBottom + drawTileHeight;

        int screenLeft = worldToScreenX(worldLeft);
        int screenRight = worldToScreenX(worldRight);
        int screenTop = worldToScreenY(worldTop);
        int screenBottom = worldToScreenY(worldBottom);

        int screenTileW = screenRight - screenLeft;
        int screenTileH = screenBottom - screenTop;

        if (screenTileW < 1)
        {
            screenTileW = 1;
        }

        if (screenTileH < 1)
        {
            screenTileH = 1;
        }

        putimage_alpha_tile(
            screenLeft,
            screenTop,
            screenTileW,
            screenTileH,
            &tileset,
            srcX,
            srcY,
            sourceTileWidth,
            sourceTileHeight
        );
    }

    // 功能：判断指定 tile 是否作为 solid tile 参与调试碰撞显示。
    bool isSolidTile(int row, int col)
    {
        if (row < 0 || row >= rows || col < 0 || col >= cols)
        {
            return false;
        }

        int tileId = tiles[row][col];

        // 第一版先简单处理：
        // 只要不是空格，就当作有碰撞
        if (tileId != TILE_EMPTY)
        {
            return true;
        }

        return false;
    }

    // 功能：获取指定 tile 在世界坐标中的矩形范围。
    RectBox getTileWorldBox(int row, int col)
    {
        RectBox box;

        double worldLeft = offsetX + col * drawTileWidth;
        double worldBottom = offsetY + (rows - 1 - row) * drawTileHeight;

        box.left = worldLeft;
        box.right = worldLeft + drawTileWidth;
        box.bottom = worldBottom;
        box.top = worldBottom + drawTileHeight;

        return box;
    }

    // 功能：绘制所有 solid tile 的调试碰撞框。
    void drawDebugCollisionBoxes()
    {
        setlinecolor(YELLOW);

        for (int row = 0; row < rows; row++)
        {
            for (int col = 0; col < cols; col++)
            {
                if (!isSolidTile(row, col))
                {
                    continue;
                }

                RectBox box = getTileWorldBox(row, col);

                int screenLeft = worldToScreenX(box.left);
                int screenRight = worldToScreenX(box.right);
                int screenTop = worldToScreenY(box.top);
                int screenBottom = worldToScreenY(box.bottom);

                rectangle(screenLeft, screenTop, screenRight, screenBottom);
            }
        }
    }
    // 功能：获取当前 tile map 对应的世界宽度。
    int getworldWidth()//根据列数和每个 tile 的显示宽度计算整个地图在世界坐标系中的宽度
    {
        return cols * drawTileWidth * 3;
    }
    // 功能：获取当前 tile map 对应的世界高度。
    int getWOrldHeight()//根据行数和每个 tile 的显示高度计算整个地图在世界坐标系中的高度
    {
        return rows * drawTileHeight * 3;
    }
};
// UIAnchor：
 // UI 锚点枚举。用于把 UIBox 放置到窗口四角或中心。
enum UIAnchor
{
    UI_TOP_LEFT,
    UI_TOP_RIGHT,
    UI_BOTTOM_LEFT,
    UI_BOTTOM_RIGHT,
    UI_CENTER
};
// UIBox：
 // 屏幕空间下的 UI 矩形框，x/y 是左上角，w/h 是宽高。
struct UIBox
{
    int x;
    int y;
    int w;
    int h;
};
//基于锚点的放置ui盒子，锚点为视口的边
// 功能：根据锚点和边距计算 UI 矩形在屏幕上的位置。
UIBox makeUIBoxByAnchor(
    int w,
    int h,
    UIAnchor anchor,
    int marginX,//定义marginX和marginY变量以控制与左上角边框的间距
    int marginY
)
{
    UIBox box;

    box.w = w;
    box.h = h;
    //定义锚点：
    //左上
    if (anchor == UI_TOP_LEFT)
    {
        box.x = marginX;
        box.y = marginY;
    }
    //右上
    else if (anchor == UI_TOP_RIGHT)
    {
        box.x = WINDOW_WIDTH - w - marginX;
        box.y = marginY;
    }
    //左下
    else if (anchor == UI_BOTTOM_LEFT)
    {
        box.x = marginX;
        box.y = WINDOW_HEIGHT - h - marginY;
    }
    //右下
    else if (anchor == UI_BOTTOM_RIGHT)
    {
        box.x = WINDOW_WIDTH - w - marginX;
        box.y = WINDOW_HEIGHT - h - marginY;
    }
    //居中
    else
    {
        box.x = WINDOW_WIDTH / 2 - w / 2;
        box.y = WINDOW_HEIGHT / 2 - h / 2;
    }

    return box;
}
// SmoothUIPanel：
 // 一个临时 UI 面板结构，支持锚点切换和平滑移动。
 // 平滑移动公式：
 //   x += (targetX - x) * moveSpeed
 //   y += (targetY - y) * moveSpeed
 // 这和 Camera 的平滑跟随是同类 lerp 思想。
struct SmoothUIPanel
{
    double x;
    double y;

    double targetX;
    double targetY;

    int w;
    int h;

    int marginX;
    int marginY;

    UIAnchor targetAnchor;

    double moveSpeed;

    // 功能：初始化平滑 UI 面板的默认状态。
    SmoothUIPanel()
    {
        x = 0;
        y = 0;

        targetX = 0;
        targetY = 0;

        w = 0;
        h = 0;

        marginX = 0;
        marginY = 0;

        targetAnchor = UI_TOP_LEFT;

        moveSpeed = 0.2;
    }

    // 功能：设置 UI 面板尺寸、锚点和初始位置。
    void init(int newW, int newH, UIAnchor startAnchor, int newMarginX, int newMarginY)
    {
        w = newW;
        h = newH;

        marginX = newMarginX;
        marginY = newMarginY;

        targetAnchor = startAnchor;

        UIBox startBox = makeUIBoxByAnchor(w, h, startAnchor, marginX, marginY);

        x = startBox.x;
        y = startBox.y;

        targetX = startBox.x;
        targetY = startBox.y;
    }

    // 功能：切换 UI 面板目标锚点。
    void setAnchor(UIAnchor newAnchor)
    {
        targetAnchor = newAnchor;

        UIBox targetBox = makeUIBoxByAnchor(w, h, targetAnchor, marginX, marginY);

        targetX = targetBox.x;
        targetY = targetBox.y;
    }

    // 功能：平滑推进 UI 面板当前位置，使其靠近目标位置。
    void update()
    {
        x += (targetX - x) * moveSpeed;
        y += (targetY - y) * moveSpeed;

        // 防止最后因为小数无限接近但不到达
        if (fabs(targetX - x) < 0.1)
        {
            x = targetX;
        }

        if (fabs(targetY - y) < 0.1)
        {
            y = targetY;
        }
    }

    // 功能：获取当前 UI 面板的屏幕矩形。
    UIBox getBox()
    {
        UIBox box;

        box.x = (int)x;
        box.y = (int)y;
        box.w = w;
        box.h = h;

        return box;
    }
};

// drawUIBox：
 // 绘制一个圆角 UI 矩形。当前用于调试面板和面板内的临时条目。
// 功能：绘制一个指定颜色的圆角 UI 矩形。
void drawUIBox(UIBox box, COLORREF fillColor, COLORREF borderColor)
{
    setfillcolor(fillColor);
    setlinecolor(borderColor);

    fillroundrect(
        box.x,
        box.y,
        box.x + box.w,
        box.y + box.h,
        30,
        30
    );
}
// drawListPanel：
 // 临时绘制一个列表面板，用于测试 UI 锚点、平滑移动和简单层级绘制。
// 功能：绘制当前用于测试的列表面板 UI。
void drawListPanel(UIBox panel)
{
    drawUIBox(panel, RGB(255, 255, 255), WHITE);

    int padding = 16;
    int itemH = 36;
    int gap = 8;

    for (int i = 0; i < 7; i++)
    {
        UIBox item;

        item.x = panel.x + padding;
        item.y = panel.y + padding + i * (itemH + gap);
        item.w = 4 * itemH;
        item.h = itemH;

        drawUIBox(item, RGB(255, 0, 0), RGB(180, 180, 180));
    }

    UIBox button1;
    button1.x = panel.x + padding;
    button1.y = panel.y + panel.h - padding - 40;
    button1.w = 120;
    button1.h = 40;

    drawUIBox(button1, RGB(90, 90, 90), WHITE);

    UIBox button2;
    button2.x = button1.x + button1.w + 12;
    button2.y = button1.y;
    button2.w = 120;
    button2.h = 40;

    drawUIBox(button2, RGB(90, 90, 90), WHITE);
}

// InputManager：
 // 统一采集键盘和鼠标输入。
 // keyNow 表示当前帧是否按下；keyLast 表示上一帧是否按下。
 // 因此可以得到三种状态：
 //   isKeyDown     = 当前帧按下
 //   isKeyPressed  = 当前帧按下 && 上一帧没按下，即“刚按下”
 //   isKeyReleased = 当前帧没按下 && 上一帧按下，即“刚松开”
 // 这样 Entity 不再直接调用 GetAsyncKeyState，从而实现输入层解耦。
class InputManager
{
private:
    bool keyNow[256];
    bool keyLast[256];

    bool mouseLeftNow;
    bool mouseLeftLast;

    int mouseX;
    int mouseY;

public:
    // 功能：初始化键盘和鼠标输入缓存。
    InputManager()
    {
        for (int i = 0; i < 256; i++)
        {
            keyNow[i] = false;
            keyLast[i] = false;
        }

        mouseLeftNow = false;
        mouseLeftLast = false;

        mouseX = WINDOW_WIDTH / 2;
        mouseY = WINDOW_HEIGHT / 2;
    }

    // 功能：刷新键盘、鼠标当前帧和上一帧输入状态。
    void update()
    {
        for (int i = 0; i < 256; i++)
        {
            keyLast[i] = keyNow[i];
            keyNow[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
        }

        mouseLeftLast = mouseLeftNow;

        ExMessage msg;

        while (peekmessage(&msg, EX_MOUSE))
        {
            if (msg.message == WM_MOUSEMOVE)
            {
                mouseX = msg.x;
                mouseY = msg.y;
            }

            if (msg.message == WM_LBUTTONDOWN)
            {
                mouseLeftNow = true;
            }

            if (msg.message == WM_LBUTTONUP)
            {
                mouseLeftNow = false;
            }
        }
    }

    // 功能：判断指定按键当前是否处于按下状态。
    bool isKeyDown(int key)
    {
        return keyNow[key];
    }

    // 功能：判断指定按键是否在当前帧刚刚按下。
    bool isKeyPressed(int key)
    {
        return keyNow[key] && !keyLast[key];
    }

    // 功能：判断指定按键是否在当前帧刚刚松开。
    bool isKeyReleased(int key)
    {
        return !keyNow[key] && keyLast[key];
    }

    // 功能：判断鼠标左键当前是否处于按下状态。
    bool isMouseLeftDown()
    {
        return mouseLeftNow;
    }

    // 功能：判断鼠标左键是否在当前帧刚刚按下。
    bool isMouseLeftPressed()
    {
        return mouseLeftNow && !mouseLeftLast;
    }

    // 功能：判断鼠标左键是否在当前帧刚刚松开。
    bool isMouseLeftReleased()
    {
        return !mouseLeftNow && mouseLeftLast;
    }

    // 功能：获取鼠标当前屏幕 X 坐标。
    int getMouseX()
    {
        return mouseX;
    }

    // 功能：获取鼠标当前屏幕 Y 坐标。
    int getMouseY()
    {
        return mouseY;
    }

    // 功能：获取鼠标相对窗口中心的 X 偏移。
    int getMouseOffsetX()
    {
        return mouseX - WINDOW_WIDTH / 2;
    }

    // 功能：获取鼠标相对窗口中心的 Y 偏移。
    int getMouseOffsetY()
    {
        return mouseY - WINDOW_HEIGHT / 2;
    }
};
// BehaviorIntent：
 // 行为意图数据包，只描述“这一帧想做什么”，不描述“具体怎么做”。
 // 例如：
 //   moveX = 1        表示想向右移动
 //   wantJump = true  表示想跳跃
 //   wantSprint=true  表示想冲刺
 // 它不应该保存 velocity、allowedMove、onGround 等物理计算结果。
struct BehaviorIntent
{
    double moveX;
    double moveY;

    bool wantJump;
    bool wantSprint;
    bool wantInteract;

    // 功能：初始化一帧空的行为意图。
    BehaviorIntent()
    {
        moveX = 0;
        moveY = 0;

        wantJump = false;
        wantSprint = false;
        wantInteract = false;
    }
};

// PlayerController：
 // 负责把 InputManager 中的原始输入翻译成 BehaviorIntent。
 // 也就是：
 //   键盘状态 -> 玩家意图
 // 它不负责修改 Entity 坐标，也不负责碰撞、动画、物理。
 // 后续 AIController、SequenceController 也可以生成同样的 BehaviorIntent。
class PlayerController
{
public:
    // 功能：把玩家输入转换为本帧行为意图。
    BehaviorIntent makeIntent(InputManager& input, bool god)
    {
        BehaviorIntent intent;

        if (input.isKeyDown(VK_LEFT))
        {
            intent.moveX = -1;
        }

        if (input.isKeyDown(VK_RIGHT))
        {
            intent.moveX = 1;
        }

        if (god)
        {
            if (input.isKeyDown(VK_UP))
            {
                intent.moveY = 1;
            }

            if (input.isKeyDown(VK_DOWN))
            {
                intent.moveY = -1;
            }
        }

        intent.wantSprint = input.isKeyDown(VK_SHIFT);
        intent.wantJump = input.isKeyPressed(VK_SPACE);
        intent.wantInteract = input.isKeyPressed('E');

        return intent;
    }
};

//前置声明
// 前置声明
class Entity;

// CollisionHandle：
 // 碰撞底层能力提供者。
 // 主要职责：
 //   1. 判断两个 AABB 是否重叠：isRectOverlapping
 //   2. 判断一维范围是否重叠：isRangeOverlapping
 //   3. 根据期望位移计算允许位移：getAllowedMoveX/Y
 //   4. 把实体限制在世界边界内：limitInWorld
 //
 // 阻挡碰撞和重叠事件的底层都和 AABB / overlap 有关，
 // 但是用途不同：
 //   - 重叠事件：关心“两个盒子现在有没有重叠”
 //   - 阻挡移动：关心“想移动这么多，最多允许移动多少”
 //
 // 当前 getAllowedMoveX/Y 并不是简单地“把实体移动到下一帧再判断是否 overlap”，
 // 而是用当前碰撞盒 + 移动方向 + 另一轴范围重叠，计算离最近阻挡物还有多远。
 // 它本质上是在“阻止下一帧发生 overlap”。
class CollisionHandle
{
public:
    // 功能：声明两个 AABB 矩形重叠检测接口。
    bool isRectOverlapping(RectBox a, RectBox b);

    // 功能：声明两个一维区间重叠检测接口。
    bool isRangeOverlapping(
        double aMin,
        double aMax,
        double bMin,
        double bMax
    );

    // 功能：声明 X 轴允许位移计算接口。
    double getAllowedMoveX(
        Entity& self,
        double moveX,
        Entity entitys[],
        int entityCount,
        int selfIndex
    );

    // 功能：声明 Y 轴允许位移计算接口。
    double getAllowedMoveY(
        Entity& self,
        double moveY,
        Entity entitys[],
        int entityCount,
        int selfIndex
    );

    // 功能：声明实体世界边界限制接口。
    void limitInWorld(
        Entity& self,
        int worldWidth,
        int worldHeight
    );
};


// MovementHandle：
 // 移动/物理执行系统。
 // 它读取 BehaviorIntent，计算速度、冲刺、跳跃、重力和期望位移。
 // 它不亲自判断碰撞细节，而是把 wantMoveX/wantMoveY 交给 CollisionHandle，
 // 得到 allowedMoveX/allowedMoveY 后，再把结果写回 Entity。
 //
 // 重要公式：
 //   currentSpeed = speed 或 speed * 2
 //   wantMoveX = inputX * currentSpeed
 //   velocityY -= GRAVITY
 //   wantMoveY = velocityY
 //   actualMove = allowedMove
class MovementHandle
{
public:
    // 功能：声明实体移动与物理更新接口。
    void update(
        Entity& self,
        BehaviorIntent intent,
        Entity entitys[],
        int entityCount,
        int selfIndex,
        int worldWidth,
        int worldHeight,
        CollisionHandle& collisionHandle
    );
};// 实体类

// Entity：
 // 实体数据容器 + 少量表现接口。
 // 旧的移动/阻挡/边界修正逻辑已经迁移到 MovementHandle 和 CollisionHandle。
 // 因此 Entity 不再自己执行移动物理，只保存状态并提供碰撞盒、动画、绘制等接口。
 //
 // Entity 当前主要保存：
 //   - 世界坐标 x/y
 //   - 速度 speed / velocityY
 //   - 状态 onGround / InAir / sprinting / jumping / isAlive
 //   - 碰撞配置 collisionBox
 //   - 动画 animation
 //   - 类型 entityType
class Entity
{
    //声明友元使MovementHandle可以访问entity的私有数据
    friend class MovementHandle;
    friend class CollisionHandle;
private:
    animatedSprite animation;
    // 世界坐标，左下角原点
    // x / y 表示实体中心点
    double x;
    double y;

    double speed;
    double velocityY;

    bool controlled;       // 是否由键盘控制
    bool collidable;       // 是否参与重叠事件检测
    bool blocking;         // 是否阻挡其它实体
    bool god;              // 是否为 god，god 不受重力和物理碰撞影响

    bool overlapping;      // 是否与可碰撞对象真正重叠
    bool collisionState;   // 是否发生碰撞状态，用于控制碰撞箱颜色
    bool InAir;          // 是否在空中
    bool onGround;         // 是否站在地面或平台上
    bool sprinting;        // 是否正在冲刺
    bool jumping;           //是否跳跃
    bool blockedByEntity;  // 本帧是否被实体阻挡
    bool blockedByWorld;   // 本帧是否被世界边界阻挡
    //做临时用，添加实体类型标签
    EntityType entityType;

    //实体是否存活
    bool isAlive;

    CollisionBox collisionBox;
    AnimationState currentAnimState;//记录当前的动画状态
    facingDirection currentFacingDirection;//记录当前操作的有效朝向
public:
    // 功能：初始化一个默认实体及其基础状态。
    Entity()
    {
        x = 0;
        y = 0;
        speed = 5;
        speed = 5;
        velocityY = 0;

        controlled = false;
        collidable = false;
        blocking = false;
        god = true;

        overlapping = false;
        collisionState = false;

        onGround = false;
        sprinting = false;
        InAir = false;
        jumping = false;
        blockedByEntity = false;
        blockedByWorld = false;

        collisionBox.width = 0;
        collisionBox.height = 0;
        collisionBox.offsetX = 0.0;
        collisionBox.offsetY = 0.0;
        collisionBox.scaleX = 1.0;
        collisionBox.scaleY = 1.0;
        //默认构造定义entity的动画状态
        currentAnimState = ANIM_COUNT;
        currentFacingDirection = LEFT;

        entityType = DEFAULT;
        isAlive = 1;
    }

    // 功能：按资源路径和初始属性创建一个可用实体。
    Entity(
        const TCHAR* imagePath,
        double startX,
        double startY,
        bool isControlled,
        bool isCollidable,
        bool isBlocking,
        bool isGod,
        EntityType Type = DEFAULT,
        int frameCount = 1,
        bool alive = 1
    )
    {
        animation.load(imagePath, frameCount);
        animation.setSpeed(4);
        animation.setLoop(true);

        x = startX;
        y = startY;

        speed = 5;
        velocityY = 0;

        controlled = isControlled;
        collidable = isCollidable;
        blocking = isBlocking;
        god = isGod;

        overlapping = false;
        collisionState = false;

        onGround = false;
        sprinting = false;
        InAir = false;
        jumping = false;
        blockedByEntity = false;
        blockedByWorld = false;

        int imgW = animation.getFrameWidth();
        int imgH = animation.getFrameHeight();

        collisionBox.width = imgW;
        collisionBox.height = imgH;
        collisionBox.offsetX = 0.0;
        collisionBox.offsetY = 0.0;
        collisionBox.scaleX = 1.0;
        collisionBox.scaleY = 1.0;

        //有参构造定义entity的动画状态

        currentAnimState = ANIM_COUNT;
        currentFacingDirection = LEFT;

        entityType = Type;
        isAlive = alive;
    }
    // 功能：获取实体类型标签。
    EntityType getEntityType()
    {
        return entityType;
    }
    // 功能：判断实体是否参与重叠事件检测。
    bool isCollidable()//获取是否可碰撞检测
    {
        return collidable;
    }

    // 功能：判断实体是否作为阻挡物参与移动修正。
    bool isBlocking()//获取是否可被阻挡
    {
        return blocking;
    }

    // 功能：判断实体是否处于 god 模式。
    bool isGod()//获取是否为god
    {
        return god;
    }

    // 功能：判断实体当前是否站在地面或平台上。
    bool isOnGround()//获取是否在地上
    {
        return onGround;
    }
    // 功能：判断实体当前是否处于空中。
    bool isInAir()//获取是否在空中
    {
        return InAir;
    }
    // 功能：判断实体当前是否正在冲刺。
    bool isSprinting()//获取是否在奔跑
    {
        return sprinting;
    }
    // 功能：判断实体当前是否处于跳跃状态。
    bool isJumping()//获取是否在跳跃
    {
        return jumping;
    }
    // 功能：判断实体本帧是否处于碰撞或重叠反馈状态。
    bool hasCollisionState()//获取碰撞状态
    {
        return collisionState;
    }

    // 功能：判断实体本帧是否被其它实体阻挡。
    bool isBlockedByEntity()//获取是否被阻挡
    {
        return blockedByEntity;
    }

    // 功能：判断实体本帧是否被世界边界阻挡。
    bool isBlockedByWorld()//获取是否被世界边界阻挡
    {
        return blockedByWorld;
    }
    // 功能：获取实体当前是否存活。
    bool getIsAlive()
    {
        return isAlive;
    }

    // 功能：设置实体存活状态。
    void setIsAlive(bool value)
    {
        isAlive = value;
    }
    // 功能：将实体标记为死亡。
    void killEntity()
    {
        this->isAlive = 0;

    }
    // 功能：获取实体中心点的世界 X 坐标。
    double getX()//获取实体的世界坐标 X，注意这里返回的是实体中心点的坐标
    {
        return x;

    }
    // 功能：获取实体中心点的世界 Y 坐标。
    double getY()//获取实体的世界坐标 Y，注意这里返回的是实体中心点的坐标
    {
        return y;
    }
    // 功能：设置实体本帧重叠状态并触发碰撞反馈显示。
    void setOverlapping(bool value)//设置重叠状态
    {
        overlapping = value;

        if (value)
        {
            collisionState = true;
        }
    }

    // 功能：直接设置实体碰撞反馈状态。
    void setCollisionState(bool value)//设置碰撞状态
    {
        collisionState = value;
    }

    // 功能：清理实体每帧临时状态。
    void clearFrameState()//每帧需要清除/重新更新的状态
    {
        overlapping = false;
        collisionState = false;

        //sprinting = false;

        blockedByEntity = false;
        blockedByWorld = false;

        // 这里不要清除 onGround。
        // onGround 是物理状态，不是单帧显示状态。
        // 它会在 update() 里的垂直运动阶段重新判断。
    }


    //将定义的碰撞盒转换为坐标系下的实际范围，用作碰撞检测
    // 功能：根据指定中心点计算实体碰撞盒的世界坐标范围。
    RectBox getWorldCollisionBoxAt(double testX, double testY)
    {
        // 根据某个测试坐标生成“世界坐标下的真实 AABB”。
        // 公式：
        //   colliderCenter = entityCenter + collisionOffset
        //   colliderWidth  = baseWidth  * scaleX
        //   colliderHeight = baseHeight * scaleY
        //   left   = centerX - width / 2
        //   right  = centerX + width / 2
        //   bottom = centerY - height / 2
        //   top    = centerY + height / 2
        RectBox box;

        double colliderCenterX = testX + collisionBox.offsetX;
        double colliderCenterY = testY + collisionBox.offsetY;

        double colliderWidth = collisionBox.width * collisionBox.scaleX;
        double colliderHeight = collisionBox.height * collisionBox.scaleY;

        box.left = colliderCenterX - colliderWidth / 2.0;
        box.right = colliderCenterX + colliderWidth / 2.0;
        box.bottom = colliderCenterY - colliderHeight / 2.0;
        box.top = colliderCenterY + colliderHeight / 2.0;

        return box;
    }

    // 功能：获取实体当前位置下的世界碰撞盒。
    RectBox getWorldCollisionBox()
    {
        return getWorldCollisionBoxAt(x, y);
    }


    // 功能：设置实体碰撞盒缩放比例。
    void setCollisionScale(double scaleX, double scaleY)
    {
        collisionBox.scaleX = scaleX;
        collisionBox.scaleY = scaleY;
    }
    // 功能：切换玩家动画状态并加载对应序列帧资源。
    void changeAnimation(AnimationState newState)
    {
        if (currentAnimState == newState)
        {
            return;
        }

        currentAnimState = newState;

        if (newState == ANIM_IDLE_L)
        {
            animation.load(_T("assets\\tex\\entities\\characters\\player1_idle_L.png"), 8);
        }
        else if (newState == ANIM_IDLE_R)
        {
            animation.load(_T("assets\\tex\\entities\\characters\\player1_idle_R.png"), 8);
        }
        else if (newState == ANIM_WALK_LEFT)
        {
            animation.load(_T("assets\\tex\\entities\\characters\\player1_walk_L.png"), 8);
        }
        else if (newState == ANIM_WALK_RIGHT)
        {
            animation.load(_T("assets\\tex\\entities\\characters\\player1_walk_R.png"), 8);
        }
        else if (newState == ANIM_RUN_LEFT)
        {
            animation.load(_T("assets\\tex\\entities\\characters\\player1_run_L.png"), 8);
        }
        else if (newState == ANIM_RUN_RIGHT)
        {
            animation.load(_T("assets\\tex\\entities\\characters\\player1_run_R.png"), 8);
        }
        animation.setSpeed(3);
        animation.setLoop(true);
    }
    //更新动画状态by意图
    // 功能：根据行为意图和冲刺状态更新实体动画。
    void updateAnimationByIntent(BehaviorIntent intent)
    {
        if (!controlled)
        {
            return;
        }

        double inputX = intent.moveX;

        if (inputX < 0)
        {
            currentFacingDirection = LEFT;

            if (sprinting)
            {
                changeAnimation(ANIM_RUN_LEFT);
            }
            else
            {
                changeAnimation(ANIM_WALK_LEFT);
            }
        }
        else if (inputX > 0)
        {
            currentFacingDirection = RIGHT;

            if (sprinting)
            {
                changeAnimation(ANIM_RUN_RIGHT);
            }
            else
            {
                changeAnimation(ANIM_WALK_RIGHT);
            }
        }
        else
        {
            if (currentFacingDirection == LEFT)
            {
                changeAnimation(ANIM_IDLE_L);
            }

            if (currentFacingDirection == RIGHT)
            {
                changeAnimation(ANIM_IDLE_R);
            }
        }
    }
    // 功能：推进实体当前动画帧。
    void updateAnimatedSprite()
    {
        animation.update();
    }



    // 渲染




	// 功能：绘制实体当前动画画面。
	void drawSprite()
	{
		animation.draw(x, y);
	}

	// 功能：绘制实体调试碰撞框。
	void drawDebugCollisionBox()
	{
		drawCollisionBox();
	}

	// 功能：按旧接口绘制实体画面和调试碰撞框。
	void draw()
	{
		drawSprite();
		drawDebugCollisionBox();
	}
    // 功能：根据实体碰撞状态绘制红色或绿色碰撞盒。
    void drawCollisionBox()
    {
        RectBox box = getWorldCollisionBox();

        // 碰撞箱颜色只跟碰撞状态有关：
        // 绿色：正常
        // 红色：本帧发生碰撞状态
        if (collisionState)
        {
            setlinecolor(RED);
        }
        else
        {
            setlinecolor(GREEN);
        }

        int screenLeft = worldToScreenX(box.left);
        int screenRight = worldToScreenX(box.right);

        int screenTop = worldToScreenY(box.top);
        int screenBottom = worldToScreenY(box.bottom);

        rectangle(screenLeft, screenTop, screenRight, screenBottom);
    }
    // 功能：设置实体 sprite 绘制缩放和偏移。
    void setSpriteTransform(double scaleX, double scaleY, double offsetX, double offsetY)
    {
        animation.setTransform(scaleX, scaleY, offsetX, offsetY);

    }
    // 功能：设置实体动画播放速度。
    void setAnimationSpeed(int speed)
    {
        animation.setSpeed(speed);
    }

};

//声音播放支持，声音当然也跟sprite等类似是一个单独的类，也具有多种状态

// 功能：根据行为意图、物理规则和碰撞结果更新实体移动状态。
void MovementHandle::update(
    Entity& self,
    BehaviorIntent intent,
    Entity entitys[],
    int entityCount,
    int selfIndex,
    int worldWidth,
    int worldHeight,
    CollisionHandle& collisionHandle
)
{
    /*
    MovementHandle 本帧数据流：
        BehaviorIntent
            -> inputX/inputY
            -> sprinting/currentSpeed
            -> jump/velocityY
            -> wantMoveX/wantMoveY
            -> CollisionHandle 计算 allowedMoveX/allowedMoveY
            -> 写回 self.x/self.y/self.velocityY/self.onGround 等状态

    注意：
        MovementHandle 负责“想怎么动”和“把结果应用到实体”；
        CollisionHandle 负责“这个移动是否会被阻挡、最多能走多少”。
    */
    double inputX = intent.moveX;
    double inputY = intent.moveY;

    double currentSpeed = self.speed;

    bool hasMoveInput = false;

    if (inputX != 0)
    {
        hasMoveInput = true;
    }

    bool wantSprint = false;

    if (intent.wantSprint && hasMoveInput)
    {
        wantSprint = true;
    }

    if (!wantSprint)
    {
        self.sprinting = false;
    }
    else
    {
        if (!self.sprinting && self.onGround)
        {
            self.sprinting = true;
        }

        // 如果 sprinting 本来就是 true，就允许它在空中继续保持
    }

    if (self.sprinting)
    {
        currentSpeed = self.speed * 2;
    }

    // god 模式：不受重力、不受阻挡碰撞影响，可以自由移动
    if (self.god)
    {
        double length = sqrt(inputX * inputX + inputY * inputY);

        if (length != 0)
        {
            inputX = inputX / length;
            inputY = inputY / length;
        }

        self.x += inputX * currentSpeed;
        self.y += inputY * currentSpeed;

        collisionHandle.limitInWorld(self, worldWidth, worldHeight);
        return;
    }

    // 非 god 模式：启用重力、跳跃、碰撞
    if (intent.wantJump && self.onGround)
    {
        self.velocityY = JUMP_SPEED;
        self.onGround = false;
        self.InAir = true;
        self.jumping = true;
    }

    // X 轴期望位移：
    //   inputX 只表示方向：-1 左，0 不动，1 右
    //   currentSpeed 是本帧速度
    //   因为当前项目把 1 tick 当作单位时间，所以：
    //   wantMoveX = inputX * currentSpeed * 1
    double wantMoveX = inputX * currentSpeed;

    double allowedMoveX = collisionHandle.getAllowedMoveX(
        self,
        wantMoveX,
        entitys,
        entityCount,
        selfIndex
    );
    if (fabs(allowedMoveX - wantMoveX) > EPS)
    {
        self.blockedByEntity = true;
        self.collisionState = true;
    }

    self.x += allowedMoveX;

    // Y 轴速度更新：
    //   速度 velocityY 每 tick 受重力影响减小
    //   velocityY -= GRAVITY
    //   wantMoveY = velocityY * 1
    // 当前项目把每帧时间简化为 1 tick，因此位移直接使用 velocityY。
    self.velocityY -= GRAVITY;

    if (self.velocityY < MAX_FALL_SPEED)
    {
        self.velocityY = MAX_FALL_SPEED;
    }

    self.onGround = false;
    self.InAir = true;

    double wantMoveY = self.velocityY;

    double allowedMoveY = collisionHandle.getAllowedMoveY(
        self,
        wantMoveY,
        entitys,
        entityCount,
        selfIndex
    );
    if (fabs(allowedMoveY - wantMoveY) > EPS)
    {
        if (wantMoveY < 0)
        {
            self.onGround = true;
            self.InAir = false;
            self.jumping = false;
        }
        else if (wantMoveY > 0)
        {
            self.blockedByEntity = true;
            self.collisionState = true;
        }

        self.velocityY = 0;
    }

    self.y += allowedMoveY;

    collisionHandle.limitInWorld(self, worldWidth, worldHeight);
}
// 功能：判断两个 AABB 矩形是否真正重叠。
bool CollisionHandle::isRectOverlapping(RectBox a, RectBox b)
{
    if (a.right <= b.left + EPS)
    {
        return false;
    }

    if (a.left >= b.right - EPS)
    {
        return false;
    }

    if (a.top <= b.bottom + EPS)
    {
        return false;
    }

    if (a.bottom >= b.top - EPS)
    {
        return false;
    }

    return true;
}

// 功能：判断两个一维区间是否真正重叠。
bool CollisionHandle::isRangeOverlapping(
    double aMin,
    double aMax,
    double bMin,
    double bMax
)
{
    if (aMax <= bMin + EPS)
    {
        return false;
    }

    if (aMin >= bMax - EPS)
    {
        return false;
    }

    return true;
}


// 功能：计算实体在 X 轴上不会穿透阻挡物的最大允许位移。
double CollisionHandle::getAllowedMoveX(
    Entity& self,
    double moveX,
    Entity entitys[],
    int entityCount,
    int selfIndex
)
{
    /*
    X 轴阻挡修正逻辑：
        输入：moveX = 本帧期望水平位移
        输出：allowedMove = 本帧真正允许移动的水平距离

    核心思路：
        1. 先取得当前实体的 AABB：myBox
        2. 遍历所有 blocking 实体
        3. 如果 Y 轴范围不重叠，说明上下错开，不可能水平撞到，跳过
        4. 如果向右移动，只看位于右侧的障碍：
              distance = other.left - myBox.right
              allowedMove = min(allowedMove, distance)
        5. 如果向左移动，只看位于左侧的障碍：
              distance = other.right - myBox.left
              allowedMove = max(allowedMove, distance)  // moveX 为负数，所以取更接近 0 的限制值

    它和 overlap 有关，但不是简单地“先移动到下一帧再判断 overlap”。
    它是在当前帧提前计算到障碍边缘的距离，防止下一帧真正重叠。
    */
    if (moveX == 0)
    {
        return 0;
    }

    RectBox myBox = self.getWorldCollisionBox();
    double allowedMove = moveX;

    for (int i = 0; i < entityCount; i++)
    {
        if (i == selfIndex)
        {
            continue;
        }

        // 死亡实体不再参与阻挡计算。
        // 否则被 killEntity() 的阻挡物仍可能继续挡住玩家。
        if (!entitys[i].getIsAlive())
        {
            continue;
        }

        if (!entitys[i].isBlocking())
        {
            continue;
        }

        RectBox otherBox = entitys[i].getWorldCollisionBox();

        if (!this->isRangeOverlapping(myBox.bottom, myBox.top, otherBox.bottom, otherBox.top))
        {
            continue;
        }

        if (moveX > 0)
        {
            if (otherBox.left >= myBox.right - EPS)
            {
                double distance = otherBox.left - myBox.right;

                if (distance < 0)
                {
                    distance = 0;
                }

                if (distance < allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
        else if (moveX < 0)
        {
            if (otherBox.right <= myBox.left + EPS)
            {
                double distance = otherBox.right - myBox.left;

                if (distance > 0)
                {
                    distance = 0;
                }

                if (distance > allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
    }

    return allowedMove;
}
// 功能：计算实体在 Y 轴上不会穿透阻挡物的最大允许位移。
double CollisionHandle::getAllowedMoveY(
    Entity& self,
    double moveY,
    Entity entitys[],
    int entityCount,
    int selfIndex
)
{
    /*
    Y 轴阻挡修正逻辑：
        输入：moveY = 本帧期望垂直位移，通常来自 velocityY
        输出：allowedMove = 本帧真正允许移动的垂直距离

    核心思路：
        1. 先取得当前实体的 AABB：myBox
        2. 遍历所有 blocking 实体
        3. 如果 X 轴范围不重叠，说明左右错开，不可能垂直撞到，跳过
        4. 如果向上移动，只看位于上方的障碍：
              distance = other.bottom - myBox.top
              allowedMove = min(allowedMove, distance)
        5. 如果向下移动，只看位于下方的障碍：
              distance = other.top - myBox.bottom
              allowedMove = max(allowedMove, distance)  // moveY 为负数

    下落时 allowedMoveY 与 wantMoveY 不一致，通常意味着落地；
    上升时不一致，通常意味着撞到上方阻挡物。
    */
    if (moveY == 0)
    {
        return 0;
    }

    RectBox myBox = self.getWorldCollisionBox();
    double allowedMove = moveY;

    for (int i = 0; i < entityCount; i++)
    {
        if (i == selfIndex)
        {
            continue;
        }

        // 死亡实体不再参与阻挡计算。
        // 否则被 killEntity() 的阻挡物仍可能继续挡住玩家。
        if (!entitys[i].getIsAlive())
        {
            continue;
        }

        if (!entitys[i].isBlocking())
        {
            continue;
        }

        RectBox otherBox = entitys[i].getWorldCollisionBox();

        if (!this->isRangeOverlapping(myBox.left, myBox.right, otherBox.left, otherBox.right))
        {
            continue;
        }

        if (moveY > 0)
        {
            if (otherBox.bottom >= myBox.top - EPS)
            {
                double distance = otherBox.bottom - myBox.top;

                if (distance < 0)
                {
                    distance = 0;
                }

                if (distance < allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
        else if (moveY < 0)
        {
            if (otherBox.top <= myBox.bottom + EPS)
            {
                double distance = otherBox.top - myBox.bottom;

                if (distance > 0)
                {
                    distance = 0;
                }

                if (distance > allowedMove)
                {
                    allowedMove = distance;
                }
            }
        }
    }

    return allowedMove;
}

// 功能：把实体限制在世界边界内并修正相关物理状态。
void CollisionHandle::limitInWorld(
    Entity& self,
    int worldWidth,
    int worldHeight
)
{
    /*
    世界边界修正：
        这个函数把实体限制在 [0, worldWidth] x [0, worldHeight] 内。
        如果实体超出边界，就把它推回边界内。

    重要状态反馈：
        - 撞到世界边界时 blockedByWorld = true
        - 撞到底部边界时，视为站在地面：
              onGround = true
              InAir = false
              jumping = false
              向下速度 velocityY 清零
    */
    RectBox box = self.getWorldCollisionBox();

    if (box.left < 0)
    {
        self.x += 0 - box.left;
        self.blockedByWorld = true;
    }

    box = self.getWorldCollisionBox();

    if (box.right > worldWidth)
    {
        self.x -= box.right - worldWidth;
        self.blockedByWorld = true;
    }

    box = self.getWorldCollisionBox();

    if (box.bottom < 0)
    {
        self.y += 0 - box.bottom;
        self.blockedByWorld = true;
        self.onGround = true;
        self.InAir = false;
        self.jumping = false;

        if (self.velocityY < 0)
        {
            self.velocityY = 0;
        }
    }

    box = self.getWorldCollisionBox();

    if (box.top > worldHeight)
    {
        self.y -= box.top - worldHeight;
        self.blockedByWorld = true;

        if (self.velocityY > 0)
        {
            self.velocityY = 0;
        }
    }
}


// 当前相机跟随目标下标。
 // 这是一个临时全局变量，后续可以继续迁移到 CameraHandle 或 Level 内部。
int gCameraFollowTargetIndex = 0;



// setCameraFollowTarget：
 // 根据实体下标切换相机跟随对象。无效下标或死亡实体不会被设置为目标。
// 功能：切换相机当前跟随的实体下标。
void setCameraFollowTarget(int newTargetIndex, Entity entitys[], int entityCount)
{
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


// updateCameraFollow：
 // 每帧根据当前跟随实体位置、鼠标偏移和缩放输入更新全局 Camera。
 // 数据流：Level::updateCamera -> updateCameraFollow -> gCamera.followSmooth
// 功能：根据跟随目标、鼠标偏移和缩放输入更新相机。
void updateCameraFollow(
    Entity entitys[],
    int entityCount,
    int worldWidth,
    int worldHeight,
    int mouseOffsetX,
    int mouseOffsetY
)
{
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

    gCamera.updateZoom();

    // 鼠标引导相机偏移强度
    // 0.0 = 完全不受鼠标影响
    // 0.25 = 鼠标偏移的 25% 用于相机偏移
    // 0.5 = 更明显
    double lookStrength = 0.25;

    // 死区，避免鼠标在中心附近轻微抖动导致相机一直晃
    int deadZone = 20;

    if (abs(mouseOffsetX) < deadZone)
    {
        mouseOffsetX = 0;
    }

    if (abs(mouseOffsetY) < deadZone)
    {
        mouseOffsetY = 0;
    }

    // 屏幕像素偏移 -> 世界坐标偏移
    double offsetWorldX = mouseOffsetX / gCamera.zoom * lookStrength;

    // 注意这里要反过来：
    // 鼠标在屏幕上方时 mouseOffsetY 是负数，
    // 但是世界坐标里向上应该是正数。
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
//抽象出render类统一管理各类renderable object
class Renderer
{
private:
	bool showCollisionBox;
	bool showTileCollisionBox;

public:
	// 功能：初始化渲染器的调试绘制开关。
	Renderer()
	{
		showCollisionBox = true;
		showTileCollisionBox = false;
	}

	// 功能：设置是否绘制实体碰撞框。
	void setShowCollisionBox(bool value)
	{
		showCollisionBox = value;
	}

	// 功能：设置是否绘制 tile 碰撞框。
	void setShowTileCollisionBox(bool value)
	{
		showTileCollisionBox = value;
	}

	// 功能：切换实体碰撞框显示状态。
	void toggleCollisionBox()
	{
		showCollisionBox = !showCollisionBox;
	}

	// 功能：切换 tile 碰撞框显示状态。
	void toggleTileCollisionBox()
	{
		showTileCollisionBox = !showTileCollisionBox;
	}

	// 功能：绘制当前关卡背景图。
	void drawBackground(IMAGE& background)
	{
		putimage(0, 0, &background);
	}

	// 功能：绘制 tile map，并根据开关绘制 tile 调试碰撞框。
	void drawTileMap(TileMap& tileMap)
	{
		tileMap.draw();

		if (showTileCollisionBox)
		{
			tileMap.drawDebugCollisionBoxes();
		}
	}

	// 功能：绘制所有存活实体，并根据开关绘制实体调试碰撞框。
	void drawEntities(Entity entitys[], int entityCount)
	{
		for (int i = 0; i < entityCount; i++)
		{
			if (!entitys[i].getIsAlive())
			{
				continue;
			}

			entitys[i].drawSprite();

			if (showCollisionBox)
			{
				entitys[i].drawDebugCollisionBox();
			}
		}
	}

	// 功能：绘制当前测试 UI 面板。
	void drawUI(SmoothUIPanel& listPanel)
	{
		drawListPanel(listPanel.getBox());
	}
};

// Level：
 // 当前关卡/场景管理器。
 // 它持有当前关卡的地图、背景、实体数组、UI面板和各种 Handle。
 // 它不应该亲自写复杂的移动/碰撞细节，而是负责“调度顺序”：
 //   init()   加载关卡内容
 //   update() 每帧按顺序调度输入、实体、相机、事件、UI
 //   draw()   按层级绘制背景、地图、实体、UI
class Level
{
private:
    TileMap tileMap;
    IMAGE background;

    Entity entitys[ENTITY_COUNT];

    SmoothUIPanel listPanel;
    Renderer renderer;

    PlayerController playerController;
    MovementHandle movementHandle;
    CollisionHandle collisionHandle;

    int worldWidth;
    int worldHeight;

    bool lastOverlap[ENTITY_COUNT][ENTITY_COUNT];
    bool lastCollisionState[ENTITY_COUNT];
    bool lastGroundState[ENTITY_COUNT];
    bool lastSprintState[ENTITY_COUNT];
    bool lastInAirState[ENTITY_COUNT];
    bool lastJumpingState[ENTITY_COUNT];
    bool lastAliveState[ENTITY_COUNT];

public:
    // 功能：初始化关卡实体列表和默认世界尺寸。
    Level()
        : entitys
        {
            Entity(_T("assets\\tex\\entities\\characters\\player1_Idle_L.png"), 200, 700, true, true, true, false, PLAYER, 8, 1),

            Entity(_T("assets\\tex\\entities\\characters\\player2.png"), 600, 900, false, true, false, false, ENTITY, 1),

            Entity(_T("assets\\tex\\entities\\characters\\player3.png"), 950, 850, false, true, true, false, ENTITY, 1),

            Entity(_T("assets\\tex\\entities\\characters\\player4.png"), 1300, 650, false, true, false, false, ENTITY, 1),

            Entity(_T("assets\\tex\\entities\\items\\MonedaD.png"), 256, 256, false, true, false, true, COIN, 5, 1),

            Entity(_T("assets\\tex\\entities\\items\\MonedaP.png"), 256 + 48 + 16, 256, false, true, false, true, COIN, 5, 1),

            Entity(_T("assets\\tex\\entities\\items\\MonedaR.png"), 256 + (48 * 2) + (16 * 2), 256, false, true, false, true, COIN, 5, 1)
        }
    {
        worldWidth = WINDOW_WIDTH;
        worldHeight = WINDOW_HEIGHT;
    }

    // 功能：初始化关卡地图、背景、UI、实体设置和历史状态缓存。
    void init()
    {
        initMap();
        initBackground();
        initUI();
        initEntitySettings();
        initLastStates();
    }

    // 功能：按固定顺序更新关卡中的输入、实体、相机、事件和 UI。
    void update(InputManager& input)
    {
        /*
        Level 每帧更新顺序：
            1. 处理输入反馈（临时鼠标调试）
            2. clearEntityFrameState() 清理上一帧临时状态
            3. updateEntities() 生成 intent，并调用 MovementHandle / CollisionHandle
            4. handleCameraInput() 处理 F1-F4 跟随目标切换
            5. handleUIInput() 处理 UI 锚点切换
            6. updateCamera() 更新摄像机跟随
            7. updateDebugStates() 输出状态变化
            8. updateOverlapEvents() 处理重叠事件，如金币拾取
            9. listPanel.update() 更新 UI 过渡
        */
        if (input.isMouseLeftPressed())
        {
            cout << "Left mouse down: "
                << input.getMouseX()
                << " "
                << input.getMouseY()
                << endl;
        }

        clearEntityFrameState();

        updateEntities(input);

		handleCameraInput(input);
		handleUIInput(input);
		handleRendererInput(input);

		updateCamera(input);

        updateDebugStates();

        updateOverlapEvents();

        listPanel.update();
    }

	// 功能：委托 Renderer 绘制当前关卡画面。
	void draw()
	{
		renderer.drawBackground(background);
		renderer.drawTileMap(tileMap);
		renderer.drawEntities(entitys, ENTITY_COUNT);
		renderer.drawUI(listPanel);
	}

private:
    // 功能：加载地图资源并根据地图尺寸设置世界范围。
    void initMap()
    {
        tileMap.setTileSize(16, 16, 48, 48);
        tileMap.loadTileset(_T("assets\\tex\\maps\\tileset.png"));
        tileMap.loadFromFile("assets\\tex\\maps\\map.txt");

        worldWidth = tileMap.getworldWidth();
        worldHeight = tileMap.getWOrldHeight();

        if (worldWidth < WINDOW_WIDTH)
        {
            worldWidth = WINDOW_WIDTH;
        }

        if (worldHeight < WINDOW_HEIGHT)
        {
            worldHeight = WINDOW_HEIGHT;
        }
    }

    // 功能：加载关卡背景图片。
    void initBackground()
    {
        loadimage(&background, _T("assets\\tex\\maps\\background.jpg"));
    }

    // 功能：初始化测试 UI 面板。
    void initUI()
    {
        listPanel.init(480, 600, UI_TOP_LEFT, 32, 32);
    }

    // 功能：设置实体 sprite 缩放、动画速度和碰撞盒缩放。
    void initEntitySettings()
    {
        entitys[0].setSpriteTransform(4.0, 4.0, 0, 0);

        entitys[3].setSpriteTransform(2.0, 2.0, 0, 0);

        entitys[4].setSpriteTransform(4.0, 4.0, 0, 0);
        entitys[5].setSpriteTransform(4.0, 4.0, 0, 0);
        entitys[6].setSpriteTransform(4.0, 4.0, 0, 0);

        entitys[0].setAnimationSpeed(3);
        entitys[4].setAnimationSpeed(3);

        entitys[0].setCollisionScale(4, 4);
    }

    // 功能：初始化用于检测状态变化的历史缓存。
    void initLastStates()
    {
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            lastCollisionState[i] = false;
            lastGroundState[i] = false;
            lastSprintState[i] = false;
            lastInAirState[i] = false;
            lastJumpingState[i] = false;
            lastAliveState[i] = entitys[i].getIsAlive();

            for (int j = 0; j < ENTITY_COUNT; j++)
            {
                lastOverlap[i][j] = false;
            }
        }
    }

    // 功能：清理所有存活实体的本帧临时状态。
    void clearEntityFrameState()
    {
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            if (!entitys[i].getIsAlive())
            {
                continue;
            }

            entitys[i].clearFrameState();
        }
    }

    // 功能：为实体生成行为意图并执行移动、碰撞和动画更新。
    void updateEntities(InputManager& input)
    {
        /*
        实体更新数据流：
            对每个存活实体：
                1. 创建默认空 BehaviorIntent
                2. 如果是玩家 i == 0，则由 PlayerController 根据输入生成 intent
                3. MovementHandle 根据 intent 更新移动/物理
                4. MovementHandle 内部调用 CollisionHandle 进行阻挡修正
                5. Entity 根据 intent 和 sprinting 等状态切换动画
                6. 推进动画帧
        */
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            if (!entitys[i].getIsAlive())
            {
                continue;
            }

            BehaviorIntent intent;

            if (i == 0)
            {
                intent = playerController.makeIntent(input, entitys[i].isGod());
            }

            movementHandle.update(
                entitys[i],
                intent,
                entitys,
                ENTITY_COUNT,
                i,
                worldWidth,
                worldHeight,
                collisionHandle
            );

            entitys[i].updateAnimationByIntent(intent);
            entitys[i].updateAnimatedSprite();
        }
    }

    // 功能：处理相机跟随目标切换输入。
    void handleCameraInput(InputManager& input)
    {
        if (input.isKeyPressed(VK_F1))
        {
            setCameraFollowTarget(0, entitys, ENTITY_COUNT);
        }

        if (input.isKeyPressed(VK_F2))
        {
            setCameraFollowTarget(1, entitys, ENTITY_COUNT);
        }

        if (input.isKeyPressed(VK_F3))
        {
            setCameraFollowTarget(2, entitys, ENTITY_COUNT);
        }

        if (input.isKeyPressed(VK_F4))
        {
            setCameraFollowTarget(3, entitys, ENTITY_COUNT);
        }
    }

    // 功能：处理 UI 面板锚点切换输入。
    void handleUIInput(InputManager& input)
    {
        if (input.isKeyPressed('W'))
        {
            listPanel.setAnchor(UI_TOP_LEFT);
        }

        if (input.isKeyPressed('S'))
        {
            listPanel.setAnchor(UI_TOP_RIGHT);
        }

        if (input.isKeyPressed('A'))
        {
            listPanel.setAnchor(UI_BOTTOM_LEFT);
        }

        if (input.isKeyPressed('D'))
        {
            listPanel.setAnchor(UI_BOTTOM_RIGHT);
        }
    }

	// 功能：处理渲染器调试显示开关输入。
	void handleRendererInput(InputManager& input)
	{
		if (input.isKeyPressed(VK_F5))
		{
			renderer.toggleCollisionBox();
            cout << "Toggle entity collision box." << endl;
		}

		if (input.isKeyPressed(VK_F6))
		{
			renderer.toggleTileCollisionBox();
		}
	}

    // 功能：根据输入状态更新相机跟随。
    void updateCamera(InputManager& input)
    {
        updateCameraFollow(
            entitys,
            ENTITY_COUNT,
            worldWidth,
            worldHeight,
            input.getMouseOffsetX(),
            input.getMouseOffsetY()
        );
    }

    // 功能：检测实体状态变化并输出调试信息。
    void updateDebugStates()
    {
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            if (entitys[i].hasCollisionState() && !lastCollisionState[i])
            {
                cout << "Entity " << i << " collision state started." << endl;
            }

            lastCollisionState[i] = entitys[i].hasCollisionState();
        }

        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            bool nowAlive = entitys[i].getIsAlive();

            if (!nowAlive && lastAliveState[i])
            {
                cout << "Entity " << i << " died." << endl;
            }

            lastAliveState[i] = nowAlive;
        }

        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            if (entitys[i].isOnGround() && !lastGroundState[i])
            {
                cout << "Entity " << i << " is on ground." << endl;
            }

            lastGroundState[i] = entitys[i].isOnGround();
        }

        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            if (entitys[i].isInAir() && !lastInAirState[i])
            {
                cout << "Entity " << i << " is in air." << endl;
            }

            lastInAirState[i] = entitys[i].isInAir();
        }

        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            bool nowJumping = entitys[i].isJumping();

            if (nowJumping && !lastJumpingState[i])
            {
                cout << "Entity " << i << " started jumping." << endl;
            }

            if (!nowJumping && lastJumpingState[i])
            {
                cout << "Entity " << i << " ended jumping." << endl;
            }

            lastJumpingState[i] = nowJumping;
        }

        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            bool nowSprinting = entitys[i].isSprinting();

            if (nowSprinting && !lastSprintState[i])
            {
                cout << "Entity " << i << " started sprinting." << endl;
            }

            if (!nowSprinting && lastSprintState[i])
            {
                cout << "Entity " << i << " ended sprinting." << endl;
            }

            lastSprintState[i] = nowSprinting;
        }
    }

    // 功能：检测实体重叠事件并触发金币拾取等反馈。
    void updateOverlapEvents()
    {
        /*
        重叠事件检测：
            这里处理“已经发生重叠之后要做什么”。
            例如：玩家碰到金币 -> 播放音效 -> 金币死亡。

        它和阻挡碰撞不同：
            - 阻挡碰撞：MovementHandle 想移动，CollisionHandle 计算 allowedMove，防止穿透
            - 重叠事件：实体已经重叠，触发某种游戏事件

        当前这里复用 CollisionHandle::isRectOverlapping() 作为底层 AABB 判断。
        */
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            for (int j = i + 1; j < ENTITY_COUNT; j++)
            {
                if (!entitys[i].getIsAlive() || !entitys[j].getIsAlive())
                {
                    continue;
                }

                if (!entitys[i].isCollidable() || !entitys[j].isCollidable())
                {
                    continue;
                }

                RectBox a = entitys[i].getWorldCollisionBox();
                RectBox b = entitys[j].getWorldCollisionBox();

                bool overlapping = collisionHandle.isRectOverlapping(a, b);

                if (overlapping)
                {
                    entitys[i].setOverlapping(true);
                    entitys[j].setOverlapping(true);

                    if (!lastOverlap[i][j])
                    {
                        cout << "Entity " << i << " overlaps with Entity " << j << endl;

                        EntityType typeA = entitys[i].getEntityType();
                        EntityType typeB = entitys[j].getEntityType();

                        if (typeA == PLAYER && typeB == COIN)
                        {
                            cout << "Player picked coin: " << j << endl;

                            PlaySoundW(
                                _T("assets\\sound\\entities\\item\\coin_pickup.wav"),
                                NULL,
                                SND_ASYNC | SND_NOSTOP
                            );

                            entitys[j].killEntity();
                        }
                        else if (typeA == COIN && typeB == PLAYER)
                        {
                            cout << "Player picked coin: " << i << endl;

                            PlaySoundW(
                                _T("assets\\sound\\entities\\item\\coin_pickup.wav"),
                                NULL,
                                SND_ASYNC | SND_NOSTOP
                            );

                            entitys[i].killEntity();
                        }
                        else if (typeA == PLAYER && typeB == ENTITY)
                        {
                            cout << "Player touched normal entity: " << j << endl;
                        }
                        else if (typeA == ENTITY && typeB == PLAYER)
                        {
                            cout << "Player touched normal entity: " << i << endl;
                        }
                    }
                }

                if (!entitys[i].getIsAlive() || !entitys[j].getIsAlive())
                {
                    continue;
                }

                lastOverlap[i][j] = overlapping;
            }
        }
    }
};
// 功能：程序入口，初始化窗口并运行主游戏循环。
int main()
{
    /*
    main 现在只负责程序生命周期：
        1. 初始化窗口
        2. 创建全局输入管理器 InputManager
        3. 创建当前关卡 Level
        4. 主循环中调用 input.update()、level.update()、level.draw()
        5. 退出时释放绘图窗口

    具体关卡内容已经交给 Level 管理。
    */
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(BLACK);

    InputManager input;
    Level level;

    level.init();

    BeginBatchDraw();

    while (true)
    {
        input.update();

        if (input.isKeyDown(VK_ESCAPE))
        {
            break;
        }

        level.update(input);

        cleardevice();

        level.draw();

        FlushBatchDraw();

        Sleep(16);
    }

    EndBatchDraw();
    closegraph();

    return 0;
}
