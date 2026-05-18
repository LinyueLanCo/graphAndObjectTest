#include <graphics.h>
#include <windows.h>
#include <conio.h>
#include <cmath>
#include <iostream>
using namespace std;

#pragma comment(lib, "Msimg32.lib")

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 900;
const int ENTITY_COUNT = 4;

const double EPS = 0.001;

// 重力相关参数
const double GRAVITY = 0.6;
const double JUMP_SPEED = 18.0;
const double MAX_FALL_SPEED = -28.0;


// 坐标转换


int worldToScreenX(double worldX)
{
    return (int)worldX;
}

int worldToScreenY(double worldY)
{
    return (int)(WINDOW_HEIGHT - worldY);
}


// Alpha 透明图片绘制

//将图片扩展为单独的sprite类，以实现逻辑与渲染的业务分离，后续还可以添加图层支持与序列帧动画的支持


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

class level
{
	// 这里暂时不实现，后续会添加事件与判定的统一管理
};

//重大新增功能准备：卷轴移动/摄像机概念，渲染范围跟随玩家移动，伴随而来的是抽象出实际的游戏逻辑坐标系，与实际的窗口显示内容和坐标对应关系，以及坐标转换函数的实现




//剥离并抽象出Sprite结构体，允许后续添加图层支持与序列帧动画的支持
struct Sprite
{
    IMAGE img;
	double offsetX;
	double offsetY;

    double scaleX;
    double scaleY;

    Sprite()
    {
        offsetX = 0;
        offsetY = 0;

        scaleX = 1.0;
        scaleY = 1.0;
    
    }
	//加载逻辑与IMAGE类分离
    void load(const TCHAR* imagePath)
    {
        loadimage(&img, imagePath);
    }
    //拿到原始的长和宽
	int getOriginalWidth()
	{
		return img.getwidth();
	}
	int getOriginalHeight()
	{
		return img.getheight();
	}
    //计算缩放之后的长和宽
	int getDrawWidth()
	{
		return (int)(img.getwidth() * scaleX);
	}
	int getDrawHeight()
	{
		return (int)(img.getheight() * scaleY);
	}
	//计算偏移与缩放之后之后的渲染坐标
    void setTransform(double newScaleX, double newScaleY, double newOffsetX, double newOffsetY)
    {
        scaleX = newScaleX;
        scaleY = newScaleY;

        offsetX = newOffsetX;
        offsetY = newOffsetY;
    }
    //绘制逻辑
    void draw(double ownerX, double ownerY)
    {
		//拿到缩放之后的长和宽
        int drawW = getDrawWidth();
        int drawH = getDrawHeight();
		//计算偏移与缩放之后之后的渲染坐标
        double spriteCenterX = ownerX + offsetX;
        double spriteCenterY = ownerY + offsetY;
		//因为世界坐标系以左下角为原点，而屏幕坐标系以左上角为原点，所以y轴的偏移需要反过来
        double worldLeft = spriteCenterX - drawW / 2.0;
        double worldTop = spriteCenterY + drawH / 2.0;
		//将世界坐标转换为屏幕坐标
        int drawX = worldToScreenX(worldLeft);
        int drawY = worldToScreenY(worldTop);
		//调用之前定义的支持缩放的Alpha透明图片绘制函数
        putimage_alpha(drawX, drawY, drawW, drawH, &img);
    }
	//后续可能将渲染逻辑从sprite里剥离，放进一个专门的render类，render负责处理相机位置与坐标转换，sprite只负责提供变换后的坐标与尺寸，render来调用绘制函数进行渲染
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


// 实体类


class Player
{
private:
    Sprite sprite;

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

    bool blockedByEntity;  // 本帧是否被实体阻挡
    bool blockedByWorld;   // 本帧是否被世界边界阻挡

    bool jumpKeyWasDown;

    CollisionBox collisionBox;

public:
    Player()
    {
        x = 0;
        y = 0;
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

        blockedByEntity = false;
        blockedByWorld = false;

        jumpKeyWasDown = false;

        collisionBox.width = 0;
        collisionBox.height = 0;
		collisionBox.offsetX = 0.0;
		collisionBox.offsetY = 0.0;
		collisionBox.scaleX = 1.0;
		collisionBox.scaleY = 1.0;

    }

    Player(
        const TCHAR* imagePath,
        double startX,
        double startY,
        bool isControlled,
        bool isCollidable,
        bool isBlocking,
        bool isGod
    )
    {
        sprite.load(imagePath);

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
        blockedByEntity = false;
        blockedByWorld = false;

        jumpKeyWasDown = false;

        int imgW = sprite.getOriginalWidth();
        int imgH = sprite.getOriginalHeight();

        collisionBox.width = imgW;
        collisionBox.height = imgH;
		collisionBox.offsetX = 0.0;
		collisionBox.offsetY = 0.0;
		collisionBox.scaleX = 1.0;
		collisionBox.scaleY = 1.0;

    }

    bool isCollidable()
    {
        return collidable;
    }

    bool isBlocking()
    {
        return blocking;
    }

    bool isGod()
    {
        return god;
    }

    bool isOnGround()
    {
        return onGround;
    }
	bool isInAir()
	{
		return InAir;
	}
    bool isSprinting()
    {
        return sprinting;
    }

    bool hasCollisionState()
    {
        return collisionState;
    }

    bool isBlockedByEntity()
    {
        return blockedByEntity;
    }

    bool isBlockedByWorld()
    {
        return blockedByWorld;
    }

    void setOverlapping(bool value)
    {
        overlapping = value;

        if (value)
        {
            collisionState = true;
        }
    }

    void setCollisionState(bool value)
    {
        collisionState = value;
    }

    void clearFrameState()
    {
        overlapping = false;
        collisionState = false;

        sprinting = false;

        blockedByEntity = false;
        blockedByWorld = false;

        // 注意：
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

    
    // 更新逻辑
    

    void update(Player players[], int entityCount, int selfIndex)
    {
        double inputX = 0;
        double inputY = 0;

        if (controlled)
        {
            if (GetAsyncKeyState(VK_LEFT) & 0x8000)
            {
                inputX = -1;
            }

            if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
            {
                inputX = 1;
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

        if (shiftDown)
        {
            currentSpeed = speed * 2;
        }

        // 是否正在冲刺：
        // 必须是受控制实体，按下 Shift，并且有移动输入。
        if (controlled && shiftDown && (inputX != 0 || inputY != 0))
        {
            sprinting = true;
        }

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

            limitInWindow();

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
        }

        jumpKeyWasDown = jumpKeyDown;

        // x 轴暂时不使用加速度
        double wantMoveX = inputX * currentSpeed;

        double allowedMoveX = getAllowedMoveX(wantMoveX, players, entityCount, selfIndex);

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

        double wantMoveY = velocityY;

        double allowedMoveY = getAllowedMoveY(wantMoveY, players, entityCount, selfIndex);

        if (fabs(allowedMoveY - wantMoveY) > EPS)
        {
            // 向下时被阻挡，视为落地，不把它当作红色碰撞状态
            if (wantMoveY < 0)
            {
                onGround = true;
                InAir = false;
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

        limitInWindow();
    }

    
    // X 方向阻挡检测
    

    double getAllowedMoveX(double moveX, Player players[], int entityCount, int selfIndex)
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

            if (!players[i].isBlocking())
            {
                continue;
            }

            RectBox otherBox = players[i].getWorldCollisionBox();

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
    

    double getAllowedMoveY(double moveY, Player players[], int entityCount, int selfIndex)
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

            if (!players[i].isBlocking())
            {
                continue;
            }

            RectBox otherBox = players[i].getWorldCollisionBox();

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

    
    // 世界边界限制
    

    void limitInWindow()
    {
        RectBox box = getWorldCollisionBox();

        if (box.left < 0)
        {
            x += 0 - box.left;
            blockedByWorld = true;
        }

        box = getWorldCollisionBox();

        if (box.right > WINDOW_WIDTH)
        {
            x -= box.right - WINDOW_WIDTH;
            blockedByWorld = true;
        }

        box = getWorldCollisionBox();

        if (box.bottom < 0)
        {
            y += 0 - box.bottom;
            blockedByWorld = true;
            onGround = true;
            InAir = false;

            if (velocityY < 0)
            {
                velocityY = 0;
            }
        }

        box = getWorldCollisionBox();

        if (box.top > WINDOW_HEIGHT)
        {
            y -= box.top - WINDOW_HEIGHT;
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
		sprite.draw(x, y);

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
		sprite.setTransform(scaleX, scaleY, offsetX, offsetY);
	}
};




int main()
{
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(BLACK);
    cleardevice();

    IMAGE background;
    loadimage(&background, _T("background.jpg"));

    Player players[ENTITY_COUNT] =
    {
        // 参数：
        // 图片路径，
        // 世界坐标 x，
        // 世界坐标 y，
        // 是否受控制，
        // 是否参与重叠事件，
        // 是否阻挡移动，
        // 是否 god

        Player(_T("player1.png"), 200, 700, true, true, true, false),

        Player(_T("player2.png"), 600, 900, false, true, false, false),

        Player(_T("player3.png"), 950, 850, false, true, true, false),

        Player(_T("player4.png"), 1300, 650, false, true, false, false)

    };
    //players[0].setSpriteTransform(1.2, 1.2, 0, 20);
    //players[1].setSpriteTransform(0.8, 0.8, 0, 0);
    //players[2].setSpriteTransform(1.0, 1.0, 30, 0);
    players[3].setSpriteTransform(6.0, 6.0, 0, 160);

    bool lastOverlap[ENTITY_COUNT][ENTITY_COUNT] = {};
    bool lastCollisionState[ENTITY_COUNT] = {};
    bool lastGroundState[ENTITY_COUNT] = {};
    bool lastSprintState[ENTITY_COUNT] = {};
    bool lastInAirState[ENTITY_COUNT] = {}; 

    BeginBatchDraw();

    while (true)
    {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            break;
        }

        // 1. 清除上一帧状态
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            players[i].clearFrameState();
        }

        // 2. 更新所有实体
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            players[i].update(players, ENTITY_COUNT, i);
        }

        // 3. 输出碰撞状态变化
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            if (players[i].hasCollisionState() && !lastCollisionState[i])
            {
                cout << "Entity " << i << " collision state started." << endl;
            }

            lastCollisionState[i] = players[i].hasCollisionState();
        }

        // 4. 输出落地状态变化
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            if (players[i].isOnGround() && !lastGroundState[i])
            {
                cout << "Entity " << i << " is on ground." << endl;
            }

            lastGroundState[i] = players[i].isOnGround();
        }
		// 4. 输出起跳状态变化
        for(int i = 0; i < ENTITY_COUNT; i++)
        {
            if(players[i].isInAir() && !lastInAirState[i])
            {
                cout << "Entity " << i << " is in air." << endl;
            }
            lastInAirState[i] = players[i].isInAir();
        }
        // 5. 输出冲刺状态变化
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            if (players[i].isSprinting() && !lastSprintState[i])
            {
                cout << "Entity " << i << " started sprinting." << endl;
            }

            lastSprintState[i] = players[i].isSprinting();
        }

        // 6. 重叠事件检测,这里只检测真正重叠，贴边不算碰撞
        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            for (int j = i + 1; j < ENTITY_COUNT; j++)
            {
                if (!players[i].isCollidable() || !players[j].isCollidable())
                {
                    continue;
                }

                RectBox a = players[i].getWorldCollisionBox();
                RectBox b = players[j].getWorldCollisionBox();

                bool overlapping = isRectOverlapping(a, b);

                if (overlapping)
                {
                    players[i].setOverlapping(true);
                    players[j].setOverlapping(true);

                    if (!lastOverlap[i][j])
                    {
                        cout << "Entity " << i << " overlaps with Entity " << j << endl;
                    }
                }

                lastOverlap[i][j] = overlapping;
            }
        }

        // 7. 绘制
        cleardevice();

        putimage(0, 0, &background);

        for (int i = 0; i < ENTITY_COUNT; i++)
        {
            players[i].draw();
        }

        FlushBatchDraw();

        Sleep(16);
    }

    EndBatchDraw();
    closegraph();

    return 0;
}