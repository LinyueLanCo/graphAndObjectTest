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


// 坐标转换


//做临时用，枚举实体类型

enum EntityType
{
    PLAYER=1,
    ENTITY=2,
    COIN=3,
    DEFAULT=4
};



//镜头系统，处理逻辑绘制起点到屏幕窗口上的转换，现在支持镜头的offset和zoom，由此引申出镜头的smoothFollow也就是平滑跟随
struct Camera
{
    double x;
    double y;

    double zoom;

    double targetX;
    double targetY;
    double targetZoom;

    Camera()
    {
        x = 0;
        y = 0;

        zoom = 1.0;

        targetX = 0;
        targetY = 0;
        targetZoom = 1.0;
    }

    double getVisibleWorldWidth()
    {
        return WINDOW_WIDTH / zoom;
    }

    double getVisibleWorldHeight()
    {
        return WINDOW_HEIGHT / zoom;
    }

    void followInstant(double targetWorldX, double targetWorldY, int worldWidth, int worldHeight)
    {
        double visibleW = getVisibleWorldWidth();
        double visibleH = getVisibleWorldHeight();

        x = targetWorldX - visibleW / 2.0;
        y = targetWorldY - visibleH / 2.0;

        limitInWorld(worldWidth, worldHeight);
    }

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

    void updateZoom()
    {
        double zoomSpeed = 0.08;
        zoom += (targetZoom - zoom) * zoomSpeed;
    }

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

int worldToScreenX(double worldX)
{
    return (int)((worldX - gCamera.x) * gCamera.zoom);
}

int worldToScreenY(double worldY)
{
    return (int)(WINDOW_HEIGHT - (worldY - gCamera.y) * gCamera.zoom);
}

int worldSizeToScreen(double worldSize)
{
    return (int)(worldSize * gCamera.zoom);
}

class Sound
{



};
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
// Alpha 透明图片绘制
enum facingDirection
{
    LEFT,
    RIGHT,
    UP,
    DOWN

};


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
//重载一个版本，允许指定绘制尺寸，实现简单的缩放功能
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
//重大结构调整准备：将所有的涉及逻辑更新的事件与判定统一剥离，并抽象出level类，由level类来统一管理事件与判定，玩家类只负责输入、物理、状态更新与渲染，事件与判定的结果通过状态反馈给玩家类，由玩家类来控制状态的切换与渲染表现
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
class level
{
	// 这里暂时不实现，后续会添加事件与判定的统一管理
};


//带动画支持的sprite

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
    int getFrameWidth()
    {
        return frameWidth;
    }

    int getFrameHeight()
    {
        return frameHeight;
    }
    void setSpeed(int frameInterval)//设置帧间隔切换速度，也就是动画播放速度
    {
        if (frameInterval < 1)
        {
            frameInterval = 1;
        }
        this->frameInterval = frameInterval;
    
    }
    void setLoop(bool value)//设置是否循环
    {
        isLoop = value;
    }
    void stop()//停止播放设置
    {
        isPlaying = false;
    }
    void setTransform(double newScaleX, double newScaleY, double newOffsetX, double newOffsetY)
    {
        scaleX = newScaleX;
        scaleY = newScaleY;
        offsetX = newOffsetX;
        offsetY = newOffsetY;
    }
    void reset()
    {
        currentFrame = 0;
        frameTimer = 0;
    
    }
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


// 碰撞盒

struct RectBox
{
    double left;
    double right;
    double bottom;
    double top;
};

struct CollisionBox
{
    double width;
    double height;
    double offsetX;
	double offsetY;
	double scaleX;
	double scaleY;
};

// 矩形重叠检测：贴边不算碰撞
bool isRectOverlapping(RectBox a, RectBox b)
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

// 一维范围重叠：贴边不算重叠
bool isRangeOverlapping(double aMin, double aMax, double bMin, double bMax)
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

enum TileId
{
    TILE_EMPTY = 0
};

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

    ~TileMap()
    {
        release();
    }

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

    void setOffset(double newOffsetX, double newOffsetY)
    {
        offsetX = newOffsetX;
        offsetY = newOffsetY;
    }

    void loadTileset(const TCHAR* imagePath)
    {
        loadimage(&tileset, imagePath);
    }

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
	int getworldWidth()//根据列数和每个 tile 的显示宽度计算整个地图在世界坐标系中的宽度
	{
        return cols * drawTileWidth*3;
	}
	int getWOrldHeight()//根据行数和每个 tile 的显示高度计算整个地图在世界坐标系中的高度
    {
        return rows * drawTileHeight*3;
    }
};
//ui层
enum UIAnchor
{
    UI_TOP_LEFT,
    UI_TOP_RIGHT,
    UI_BOTTOM_LEFT,
    UI_BOTTOM_RIGHT,
    UI_CENTER
};

struct UIBox
{
    int x;
    int y;
    int w;
    int h;
};

UIBox makeUIBoxByAnchor(
    int w,
    int h,
    UIAnchor anchor,
    int marginX,
    int marginY
)
{
    UIBox box;

    box.w = w;
    box.h = h;

    if (anchor == UI_TOP_LEFT)
    {
        box.x = marginX;
        box.y = marginY;
    }
    else if (anchor == UI_TOP_RIGHT)
    {
        box.x = WINDOW_WIDTH - w - marginX;
        box.y = marginY;
    }
    else if (anchor == UI_BOTTOM_LEFT)
    {
        box.x = marginX;
        box.y = WINDOW_HEIGHT - h - marginY;
    }
    else if (anchor == UI_BOTTOM_RIGHT)
    {
        box.x = WINDOW_WIDTH - w - marginX;
        box.y = WINDOW_HEIGHT - h - marginY;
    }
    else
    {
        box.x = WINDOW_WIDTH / 2 - w / 2;
        box.y = WINDOW_HEIGHT / 2 - h / 2;
    }

    return box;
}

void drawUIBox(UIBox box, COLORREF fillColor, COLORREF borderColor)
{
    setfillcolor(fillColor);
    setlinecolor(borderColor);

    fillrectangle(
        box.x,
        box.y,
        box.x + box.w,
        box.y + box.h
    );
}

void drawListPanel()
{
    UIBox panel = makeUIBoxByAnchor(
        320,
        420,
        UI_TOP_LEFT,
        32,
        32
    );

    drawUIBox(panel, RGB(30, 30, 30), WHITE);

    int padding = 16;
    int itemH = 36;
    int gap = 8;

    for (int i = 0; i < 6; i++)
    {
        UIBox item;

        item.x = panel.x + padding;
        item.y = panel.y + padding + i * (itemH + gap);
        item.w = panel.w - padding * 2;
        item.h = itemH;

        drawUIBox(item, RGB(70, 70, 70), RGB(180, 180, 180));
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

// 实体类

//通用的entity类，目前包含了实体的所有基础逻辑处理包括碰撞输入速度处理碰撞检测等
class Entity
{
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

    bool jumpKeyWasDown;
    //做临时用，添加实体类型标签
    EntityType entityType;

    //实体是否存活
    bool isAlive;
    
    CollisionBox collisionBox;
    AnimationState currentAnimState;//记录当前的动画状态
    facingDirection currentFacingDirection;//记录当前操作的有效朝向
public:
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

        jumpKeyWasDown = false;

        collisionBox.width = 0;
        collisionBox.height = 0;
		collisionBox.offsetX = 0.0;
		collisionBox.offsetY = 0.0;
		collisionBox.scaleX = 1.0;
		collisionBox.scaleY = 1.0;

        entityType = DEFAULT;
        isAlive = 1;
    }

    Entity(
        const TCHAR* imagePath,
        double startX,
        double startY,
        bool isControlled,
        bool isCollidable,
        bool isBlocking,
        bool isGod,
        EntityType Type=DEFAULT,
        int frameCount = 1,
        bool alive=1
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

        jumpKeyWasDown = false;

        int imgW = animation.getFrameWidth();
        int imgH = animation.getFrameHeight();

        collisionBox.width = imgW;
        collisionBox.height = imgH;
        collisionBox.offsetX = 0.0;
        collisionBox.offsetY = 0.0;
        collisionBox.scaleX = 1.0;
        collisionBox.scaleY = 1.0;

        entityType = Type;
    }
    EntityType getEntityType()
    {
        return entityType;
    }
    bool isCollidable()//获取是否可碰撞检测
    {
        return collidable;
    }

    bool isBlocking()//获取是否可被阻挡
    {
        return blocking;
    }

    bool isGod()//获取是否为god
    {
        return god;
    }

    bool isOnGround()//获取是否在地上
    {
        return onGround;
    }
	bool isInAir()//获取是否在空中
	{
		return InAir;
	}
    bool isSprinting()//获取是否在奔跑
    {
        return sprinting;
    }
	bool isJumping()//获取是否在跳跃
	{
		return jumping;
	}
    bool hasCollisionState()//获取碰撞状态
    {
        return collisionState;
    }

    bool isBlockedByEntity()//获取是否被阻挡
    {
        return blockedByEntity;
    }

    bool isBlockedByWorld()//获取是否被世界边界阻挡
    {
        return blockedByWorld;
    }
    bool getIsAlive()
    {
        return isAlive;
    }

    void setIsAlive(bool value)
    {
        isAlive = value;
    }
    void killEntity()
    {
        this->isAlive = 0;
    
    }
	double getX()//获取实体的世界坐标 X，注意这里返回的是实体中心点的坐标
    {
        return x;
        
    }
	double getY()//获取实体的世界坐标 Y，注意这里返回的是实体中心点的坐标
	{
		return y;
	}
    void setOverlapping(bool value)//设置重叠状态
    {
        overlapping = value;

        if (value)
        {
            collisionState = true;
        }
    }

    void setCollisionState(bool value)//设置碰撞状态
    {
        collisionState = value;
    }

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
    RectBox getWorldCollisionBoxAt(double testX, double testY)
    {
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

    RectBox getWorldCollisionBox()
    {
        return getWorldCollisionBoxAt(x, y);
    }

    
    void setCollisionScale(double scaleX, double scaleY)
    {
        collisionBox.scaleX = scaleX;
        collisionBox.scaleY = scaleY;
    }
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
    // 更新逻辑

    void update(Entity entitys[], int entityCount, int selfIndex, int worldWidth, int worldHeight)
    {
        double inputX = 0;
        double inputY = 0;
        bool leftDown = 0;
        bool rightDown = 0;
        if (controlled)
        {
            if (GetAsyncKeyState(VK_LEFT) & 0x8000)
            {
                inputX = -1;
                leftDown = 1;
            }

            if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
            {
                inputX = 1;
                rightDown = 1;
            }


            if (god)
            {
                if (GetAsyncKeyState(VK_UP) & 0x8000)
                {
                    inputY = 1;
                }

                if (GetAsyncKeyState(VK_DOWN) & 0x8000)
                {
                    inputY = -1;
                }
            }
        }

        double currentSpeed = speed;
        bool shiftDown = false;

        if (controlled && (GetAsyncKeyState(VK_SHIFT) & 0x8000))
        {
            shiftDown = true;
        }

        bool hasMoveInput = false;

        if (inputX != 0)
        {
            hasMoveInput = true;
        }
        if (controlled)
        {
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
                if(currentFacingDirection==LEFT)
                {
                    changeAnimation(ANIM_IDLE_L);
                }
                if (currentFacingDirection == RIGHT)
                {
                    changeAnimation(ANIM_IDLE_R);
                }
            }
        }
		bool wantSprint = false;// 是否想要冲刺：

		if (controlled && shiftDown && hasMoveInput)// 冲刺条件：受控制、按下 Shift、有移动输入
        {
            wantSprint = true;
        }

        if (!wantSprint)
        {
            sprinting = false;
        }
        else
        {
            if (!sprinting && onGround)
            {
                sprinting = true;
            }

            // 如果 sprinting 本来就是 true，就允许它在空中继续保持
        }

        if (sprinting)
        {
            currentSpeed = speed * 2;
        }
        // 是否正在冲刺：
        // 必须是受控制实体，按下 Shift，并且有移动输入。


        // god 模式：不受重力、不受阻挡碰撞影响，可以自由移动
        if (god)
        {
            double length = sqrt(inputX * inputX + inputY * inputY);

            if (length != 0)
            {
                inputX = inputX / length;
                inputY = inputY / length;
            }

            x += inputX * currentSpeed;
            y += inputY * currentSpeed;

            limitInWorld(worldWidth, worldHeight);

            return;
        }

        
        // 非 god 模式：启用重力、跳跃、碰撞
        

        bool jumpKeyDown = false;

        if (controlled && (GetAsyncKeyState(VK_SPACE) & 0x8000))
        {
            jumpKeyDown = true;
        }

        if (controlled && jumpKeyDown && !jumpKeyWasDown && onGround)
        {
            velocityY = JUMP_SPEED;
            onGround = false;
            InAir = true;
			jumping = true;
        }

        jumpKeyWasDown = jumpKeyDown;

        // x 轴暂时不使用加速度
		double wantMoveX = inputX * currentSpeed;//输入决定了想要移动的距离,公式为：速度 = 输入 * 速度值，所以想要移动的距离 = 输入 * 速度值 * 时间，时间为1tick，所以简化为输入 * 速度值

        double allowedMoveX = getAllowedMoveX(wantMoveX, entitys, entityCount, selfIndex);

        if (fabs(allowedMoveX - wantMoveX) > EPS)
        {
            blockedByEntity = true;
            collisionState = true;
        }

        x += allowedMoveX;

        // y 轴使用重力
        velocityY -= GRAVITY;

        if (velocityY < MAX_FALL_SPEED)
        {
            velocityY = MAX_FALL_SPEED;
        }

        // 每次执行垂直运动之前，先假设当前不在地面
        onGround = false;
        InAir = true;

		double wantMoveY = velocityY;//速度决定了想要移动的距离，公式为：距离 = 速度 * 时间，所以想要移动的距离 = 速度 * 时间，时间为1tick，所以简化为速度值

		double allowedMoveY = getAllowedMoveY(wantMoveY, entitys, entityCount, selfIndex);//获取实际允许的移动距离

        if (fabs(allowedMoveY - wantMoveY) > EPS)
        {
            // 向下时被阻挡，视为落地，不把它当作红色碰撞状态
            if (wantMoveY < 0)
            {
                onGround = true;
                InAir = false;
				jumping = false;
            }
            else if (wantMoveY > 0)
            {
                // 向上撞到实体天花板，才算碰撞状态
                blockedByEntity = true;
                collisionState = true;
            }

            velocityY = 0;
        }

        y += allowedMoveY;

        limitInWorld(worldWidth, worldHeight);
    }
    void updateanimatedSprite()
    {
        animation.update();
    }
    
    // X 方向阻挡检测
    

    double getAllowedMoveX(double moveX, Entity entitys[], int entityCount, int selfIndex)
    {
        if (moveX == 0)
        {
            return 0;
        }

        RectBox myBox = getWorldCollisionBox();
        double allowedMove = moveX;

        for (int i = 0; i < entityCount; i++)
        {
            if (i == selfIndex)
            {
                continue;
            }

            if (!entitys[i].isBlocking())
            {
                continue;
            }

            RectBox otherBox = entitys[i].getWorldCollisionBox();

            if (!isRangeOverlapping(myBox.bottom, myBox.top, otherBox.bottom, otherBox.top))
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

    
    // Y 方向阻挡检测
    

    double getAllowedMoveY(double moveY, Entity entitys[], int entityCount, int selfIndex)
    {
        if (moveY == 0)
        {
            return 0;
        }

        RectBox myBox = getWorldCollisionBox();
        double allowedMove = moveY;

        for (int i = 0; i < entityCount; i++)
        {
            if (i == selfIndex)
            {
                continue;
            }

            if (!entitys[i].isBlocking())
            {
                continue;
            }

            RectBox otherBox = entitys[i].getWorldCollisionBox();

            if (!isRangeOverlapping(myBox.left, myBox.right, otherBox.left, otherBox.right))
            {
                continue;
            }

            if (moveY > 0)
            {
                // 障碍物在上方
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
                // 障碍物在下方
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

    
    
	//修改了世界边界限制的逻辑
    void limitInWorld(int worldWidth, int worldHeight)
    {
        RectBox box = getWorldCollisionBox();

        if (box.left < 0)
        {
            x += 0 - box.left;
            blockedByWorld = true;
        }

        box = getWorldCollisionBox();

        if (box.right > worldWidth)
        {
            x -= box.right - worldWidth;
            blockedByWorld = true;
        }

        box = getWorldCollisionBox();

        if (box.bottom < 0)
        {
            y += 0 - box.bottom;
            blockedByWorld = true;
            onGround = true;
            InAir = false;
            jumping = false;
            if (velocityY < 0)
            {
                velocityY = 0;
            }
        }

        box = getWorldCollisionBox();

        if (box.top > worldHeight)
        {
            y -= box.top - worldHeight;
            blockedByWorld = true;

            if (velocityY > 0)
            {
                velocityY = 0;
            }
        }
    }

    
    // 渲染
    



    void draw()
    {
		animation.draw(x, y);

        drawCollisionBox();
    }

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
	void setSpriteTransform(double scaleX, double scaleY, double offsetX, double offsetY)
	{
		animation.setTransform(scaleX, scaleY, offsetX, offsetY);
        
	}
    void setAnimationSpeed(int speed)
    {
        animation.setSpeed(speed);
    }
    
};

//声音播放支持，声音当然也跟sprite等类似是一个单独的类，也具有多种状态




//测试镜头跟随
int gCameraFollowTargetIndex = 0;



//设置镜头跟随目标
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


//更新镜头跟随目标
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
int main()
{

    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(BLACK);
	TileMap tileMap;
    tileMap.setTileSize(16,16,48,48);
	tileMap.loadTileset(_T("assets\\tex\\maps\\tileset.png"));
	tileMap.loadFromFile("assets\\tex\\maps\\map.txt");
    int worldWidth = tileMap.getworldWidth();
    int worldHeight = tileMap.getWOrldHeight();

    if (worldWidth < WINDOW_WIDTH)
    {
        worldWidth = WINDOW_WIDTH;
    }

    if (worldHeight < WINDOW_HEIGHT)
    {
        worldHeight = WINDOW_HEIGHT;
    }
    cleardevice();

    IMAGE background;
    loadimage(&background, _T("assets\\tex\\maps\\background.jpg"));
    Entity entitys[ENTITY_COUNT] =
    {
        // 参数：
        // 图片路径，
        // 世界坐标 x，
        // 世界坐标 y，
        // 是否受控制，
        // 是否参与重叠事件，
        // 是否阻挡移动，
        // 是否 god

        Entity(_T("assets\\tex\\entities\\characters\\player1_Idle_L.png"), 200, 700, true, true, true, false,PLAYER,8,1),

        Entity(_T("assets\\tex\\entities\\characters\\player2.png"), 600, 900, false, true, false, false,ENTITY,1),

        Entity(_T("assets\\tex\\entities\\characters\\player3.png"), 950, 850, false, true, true, false,ENTITY,1),

        Entity(_T("assets\\tex\\entities\\characters\\player4.png"), 1300, 650, false, true, false, false,ENTITY,1),
        
        Entity(_T("assets\\tex\\entities\\items\\MonedaD.png"),256,256,false,true,false,true,COIN,5,1),

        Entity(_T("assets\\tex\\entities\\items\\MonedaP.png"),256+48+16,256,false,true,false,true,COIN,5,1),

        Entity(_T("assets\\tex\\entities\\items\\MonedaR.png"),256 + (48 * 2) + (16 * 2),256,false,true,false,true,COIN,5,1)
        


    };
    entitys[0].setSpriteTransform(4.0, 4.0, 0, 0);
    //entitys[1].setSpriteTransform(0.8, 0.8, 0, 0);
    //entitys[2].setSpriteTransform(1.0, 1.0, 30, 0);
    entitys[3].setSpriteTransform(2.0, 2.0, 0, 0);
    entitys[4].setSpriteTransform(4.0, 4.0, 0, 0);
    entitys[5].setSpriteTransform(4.0, 4.0, 0, 0);
    entitys[6].setSpriteTransform(4.0, 4.0, 0, 0);
    entitys[0].setAnimationSpeed(3);
    entitys[4].setAnimationSpeed(3);
    entitys[0].setCollisionScale(4,4);

    //存储上次tick的各种状态
    bool lastOverlap[ENTITY_COUNT][ENTITY_COUNT] = {};
    bool lastCollisionState[ENTITY_COUNT] = {};
    bool lastGroundState[ENTITY_COUNT] = {};
    bool lastSprintState[ENTITY_COUNT] = {};
    bool lastInAirState[ENTITY_COUNT] = {}; 
	bool lastJumpingState[ENTITY_COUNT] = {}; 
    bool lastAliveState[ENTITY_COUNT] = {};
    int lastMouseX = 0;
    int lastMouseY = 0;
    int lastOffsetX = 0;
    int lastOffsetY = 0;
    //先更新一次所有实体的存活状态
    for (int i = 0; i < ENTITY_COUNT; i++)
    {
        lastAliveState[i] = entitys[i].getIsAlive();
    }


    int mouseX = WINDOW_WIDTH / 2;
    int mouseY = WINDOW_HEIGHT / 2;

    int mouseOffsetX = 0;
    int mouseOffsetY = 0;


    BeginBatchDraw();
    //游戏主循环
    while (true)
    {   
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
                cout << "Left mouse down: " << msg.x << " " << msg.y << endl;
            }
        }

        mouseOffsetX = mouseX - WINDOW_WIDTH / 2;
        mouseOffsetY = mouseY - WINDOW_HEIGHT / 2;
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            break;
        }

        // 1. 清除上一帧状态
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            if (!entitys[i].getIsAlive())
            {
                continue;
            }
            entitys[i].clearFrameState();
        }

        // 2. 更新所有实体
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            if (!entitys[i].getIsAlive())
            {
                continue;
            }
            
            entitys[i].update(entitys, ENTITY_COUNT, i, worldWidth, worldHeight);
            entitys[i].updateanimatedSprite();
        }
        if (GetAsyncKeyState(VK_F1) & 0x0001)
        {
            setCameraFollowTarget(0, entitys, ENTITY_COUNT);
        }

        if (GetAsyncKeyState(VK_F2) & 0x0001)
        {
            setCameraFollowTarget(1, entitys, ENTITY_COUNT);
        }

        if (GetAsyncKeyState(VK_F3) & 0x0001)
        {
            setCameraFollowTarget(2, entitys, ENTITY_COUNT);
        }

        if (GetAsyncKeyState(VK_F4) & 0x0001)
        {
            setCameraFollowTarget(3, entitys, ENTITY_COUNT);
        }

        updateCameraFollow(
            entitys,
            ENTITY_COUNT,
            worldWidth,
            worldHeight,
            mouseOffsetX,
            mouseOffsetY
        );        // 3. 输出碰撞状态变化
        for (int i = 0; i < ENTITY_COUNT; i++)
        {

            if (entitys[i].hasCollisionState() && !lastCollisionState[i])
            {
                cout << "Entity " << i << " collision state started." << endl;
            }

            lastCollisionState[i] = entitys[i].hasCollisionState();
        }
        //输出死亡变化
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            bool nowAlive = entitys[i].getIsAlive();//获取当前帧的存活状态

            if (!nowAlive && lastAliveState[i])
            {
                cout << "Entity " << i << " died." << endl;
            }

            lastAliveState[i] = nowAlive;//将当前帧的存活状态存入state里
        }
        // 4. 输出落地状态变化
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            if (entitys[i].isOnGround() && !lastGroundState[i])
            {
                cout << "Entity " << i << " is on ground." << endl;
            }

            lastGroundState[i] = entitys[i].isOnGround();
        }
		// 4. 输出是否在空中状态变化
        for(int i = 0; i < ENTITY_COUNT; i++)
        {
            if(entitys[i].isInAir() && !lastInAirState[i])
            {
                cout << "Entity " << i << " is in air." << endl;
            }
            lastInAirState[i] = entitys[i].isInAir();
        }
        //输出跳跃状态变化
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
        // 5. 输出冲刺状态变化
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

        // 6. 重叠事件检测,这里只检测真正重叠，贴边不算碰撞
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            for (int j = i + 1; j < ENTITY_COUNT; j++)
            {
                //先进行死亡检测
                if (!entitys[i].getIsAlive() || !entitys[j].getIsAlive())
                {
                    continue;
                }//再判定两者是否都是可碰撞检测
                if (!entitys[i].isCollidable() || !entitys[j].isCollidable())
                {
                    continue;
                }

                RectBox a = entitys[i].getWorldCollisionBox();
                RectBox b = entitys[j].getWorldCollisionBox();

                bool overlapping = isRectOverlapping(a, b);

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
                            PlaySoundW(_T("assets\\sound\\entities\\item\\coin_pickup.wav"), NULL,  SND_ASYNC | SND_NOSTOP);
                            entitys[j].killEntity();
                        }
                        else if (typeA == COIN && typeB == PLAYER)
                        {
                            cout << "Player picked coin: " << i << endl;
                            entitys[j].killEntity();
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

                if (!entitys[i].getIsAlive()|| !entitys[j].getIsAlive())
                {
                    continue;
                }
                lastOverlap[i][j] = overlapping;
            }
        }

        // 7. 绘制
        cleardevice();
        
        putimage(0, 0, &background);
        tileMap.draw();

        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            if (!entitys[i].getIsAlive())
            {
                continue;
            }
            entitys[i].draw();
        }
		//RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
		//drawtext(_T("Use Arrow Keys to Move, Space to Jump, Shift to Sprint, Esc to Quit"), &rect, DT_CENTER | DT_TOP | DT_SINGLELINE);


        //drawListPanel();
        FlushBatchDraw();

        Sleep(16);
    }

    EndBatchDraw();
    closegraph();




    return 0;
}