#include <graphics.h>
#include <windows.h>
#include <conio.h>
#include <cmath>
#include <iostream>
#include<fstream>
#include <vector>
// 后续会继续接入 JSON 配置读取。
using namespace std;

#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib,"winmm.lib")
const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 900;

const double EPS = 0.001;

// 重力相关参数
const double GRAVITY = 1.98;
const double JUMP_SPEED = 28.0;
const double MAX_FALL_SPEED = -28.0;

/*
============================================================
单文件临时结构顺序
============================================================

当前 game.cpp 仍然把多个模块放在同一个文件中。为了后续拆分文件，
临时按下面顺序组织：
    1. 全局常量、坐标转换、绘制辅助函数
    2. 基础数据枚举与资源描述
    3. 资源、动画播放、碰撞盒、地图、UI、输入
    4. 控制器、系统声明、Entity 声明
    5. Animator / Movement / Collision 等跨类函数实现
    6. Camera 跟随、Renderer、Level、main

后续拆文件时，可以基本按这些块拆成独立 .h / .cpp。
============================================================
*/

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
        -> Entity::updateAnimator()
        -> Animator 根据 Entity 状态切换动画
        -> Renderer 绘制实体

职责边界：
    InputManager      只负责“输入采集”
    PlayerController  只负责“输入 -> 行为意图”
    BehaviorIntent    只保存“这一帧想做什么”
    MovementHandle    负责“速度、冲刺、跳跃、重力、期望位移”
    CollisionHandle   负责“阻挡检测、允许位移、世界边界修正、重叠判断”
    Animator          负责“读取实体状态并切换动画表现”
    Level             负责“当前关卡的对象持有、初始化、更新调度和绘制调度”
    Entity            主要保存实体数据，并提供碰撞盒、动画、绘制等接口
============================================================
*/


// 坐标转换
// 当前工程使用“世界坐标”和“屏幕坐标”两套坐标：
// - 世界坐标：逻辑坐标，原点在左下角，Entity 的 x/y 表示实体中心点。
// - 屏幕坐标：EasyX 绘制坐标，原点在窗口左上角。
// - Camera 使用 centerX/centerY 记录视口中心点，并用 zoom 表示世界到屏幕的缩放倍率。
// 公式：
//   screenX = WINDOW_WIDTH / 2 + (worldX - centerX) * zoom
//   screenY = WINDOW_HEIGHT / 2 - (worldY - centerY) * zoom
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
 // centerX/centerY 表示摄像机视口中心点；zoom 表示世界到屏幕的缩放倍率。
 // followSmooth 使用 lerp 公式做平滑跟随：
 //   current += (target - current) * speed
 // 这个公式的效果是：距离目标越远移动越快，越接近目标越慢。
struct Camera
{

    // 逻辑用：摄像机真正跟随和平滑的中心点。
    // 视口边界由这个中心点和 zoom 临时推导，避免平滑缩放时左下角锚点漂移。
    double centerX;
    double centerY;

    double zoom;

    // 逻辑目标中心点。
    double targetCenterX;
    double targetCenterY;

    double targetZoom;

    // 功能：初始化相机位置、目标位置和缩放参数。
    Camera()
    {
        centerX = 0;
        centerY = 0;

        zoom = 1.0;

        targetCenterX = 0;
        targetCenterY = 0;

        targetZoom = 1.0;
    }

    // 功能：计算当前 zoom 下屏幕横向覆盖的世界宽度。
    double getVisibleWorldWidth() const
    {
        // 用固定窗口宽度除以缩放倍率，得到当前逻辑视口宽度。
        return WINDOW_WIDTH / zoom;
    }

    // 功能：计算当前 zoom 下屏幕纵向覆盖的世界高度。
    double getVisibleWorldHeight() const
    {
        // 用固定窗口高度除以缩放倍率，得到当前逻辑视口高度。
        return WINDOW_HEIGHT / zoom;
    }


    // 功能：根据中心点和可见宽度推导当前视口左边界。
    double getViewLeft() const
    {
        return centerX - getVisibleWorldWidth() / 2.0;
    }

    // 功能：根据中心点和可见宽度推导当前视口右边界。
    double getViewRight() const
    {
        return centerX + getVisibleWorldWidth() / 2.0;
    }

    // 功能：根据中心点和可见高度推导当前视口下边界。
    double getViewBottom() const
    {
        return centerY - getVisibleWorldHeight() / 2.0;
    }

    // 功能：根据中心点和可见高度推导当前视口上边界。
    double getViewTop() const
    {
        return centerY + getVisibleWorldHeight() / 2.0;
    }





    // 功能：让相机立即居中跟随目标点，并限制在世界范围内。
    void followInstant(double targetWorldX, double targetWorldY, int worldWidth, int worldHeight)
    {
        centerX = targetWorldX;
        centerY = targetWorldY;

        targetCenterX = centerX;
        targetCenterY = centerY;

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
        // 目标中心点 = 跟随实体位置 + 鼠标观察偏移。
        // 注意：这里不再减 visibleW / 2，也不再减 visibleH / 2。
        targetCenterX = targetWorldX + offsetWorldX;
        targetCenterY = targetWorldY + offsetWorldY;

        double followSpeed = 0.16;

        // 用中心点与目标中心点的差值乘以跟随系数，得到本帧平滑位移。
        centerX += (targetCenterX - centerX) * followSpeed;
        centerY += (targetCenterY - centerY) * followSpeed;

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
        // 用当前缩放和目标缩放的差值乘以速度，得到本帧缩放变化量。
        zoom += (targetZoom - zoom) * zoomSpeed;
    }

    // 将摄像机视口限制在世界范围内。
    // 这里修正的是摄像机中心点，而不是旧的视口左下角。
    // 如果世界尺寸小于可见视口，就让摄像机中心落在世界中心。
    // 功能：把相机中心限制在关卡世界边界允许的可见范围内。
    void limitInWorld(int worldWidth, int worldHeight)
    {
        double visibleW = getVisibleWorldWidth();
        double visibleH = getVisibleWorldHeight();

        double halfW = visibleW / 2.0;
        double halfH = visibleH / 2.0;
		//如果世界宽度小于等于可见宽度，摄像机中心 X 固定在世界中心；否则限制在半屏范围内。
        if (worldWidth <= visibleW)
        {
            centerX = worldWidth / 2.0;
        }
        else
        {
            if (centerX < halfW)
            {
                centerX = halfW;
            }

            if (centerX > worldWidth - halfW)
            {
                centerX = worldWidth - halfW;
            }
        }

        if (worldHeight <= visibleH)
        {
            centerY = worldHeight / 2.0;
        }
        else
        {
            if (centerY < halfH)
            {
                centerY = halfH;
            }

            if (centerY > worldHeight - halfH)
            {
                centerY = worldHeight - halfH;
            }
        }
    }

    // 功能：把世界坐标 X 转换为 EasyX 屏幕坐标 X。
    int worldToScreenX(double worldX) const
    {
        // 以屏幕中心为锚点，把世界点相对摄像机中心的距离乘以 zoom。
        return (int)(WINDOW_WIDTH / 2.0 + (worldX - centerX) * zoom);
    }

    // 功能：把世界坐标 Y 转换为 EasyX 屏幕坐标 Y。
    int worldToScreenY(double worldY) const
    {
        // EasyX 的 Y 轴向下，所以世界相对摄像机中心的 Y 偏移需要取反。
        return (int)(WINDOW_HEIGHT / 2.0 - (worldY - centerY) * zoom);
    }

    // 功能：把世界空间尺寸转换为当前缩放下的屏幕尺寸。
    int worldSizeToScreen(double worldSize) const
    {
        // 用世界尺寸乘 zoom，得到屏幕像素尺寸。
        return (int)(worldSize * zoom);
    }

};
Camera gCamera;


//修改为转发，逻辑已迁移到camera内部
// 功能：把世界坐标 X 转换为屏幕坐标 X。
int worldToScreenX(double worldX)
{
    return gCamera.worldToScreenX(worldX);
}

// 功能：把世界坐标 Y 转换为 EasyX 屏幕坐标 Y。
int worldToScreenY(double worldY)
{
    return gCamera.worldToScreenY(worldY);
}

// 功能：把世界空间尺寸转换为当前缩放下的屏幕尺寸。
int worldSizeToScreen(double worldSize)
{
    return gCamera.worldSizeToScreen(worldSize);
}

// Sound：
 // 音效系统占位类。当前项目暂时直接使用 PlaySoundW 播放音效，
 // 后续可以把音效播放、循环、停止、isPlaying 等状态统一封装进这里。
class Sound
{



};
// AnimationState：
 // 动画表现状态枚举。当前主要由 Animator 保存和切换。
 // 它描述“现在应该播放什么动画”，不代表实体真实物理状态。
enum AnimationState
{
	ANIM_IDLE_L,
	ANIM_IDLE_R,
	ANIM_WALK_LEFT,
	ANIM_WALK_RIGHT,
	ANIM_RUN_LEFT,
	ANIM_RUN_RIGHT,

	ANIM_JUMP_START_L,
	ANIM_JUMP_START_R,
	ANIM_JUMP_LOOP_L,
	ANIM_JUMP_LOOP_R,
	ANIM_JUMP_END_L,
	ANIM_JUMP_END_R,

	ANIM_COUNT
};

// AnimationSetId：
 // 动画资源组 ID，用来描述“这个实体使用哪一套动画图片资源”。
 // 它不代表实体实例编号，也不代表动画状态；AnimationState + AnimationSetId 才能定位到具体 AnimationClip。
enum AnimationSetId
{
    ANIM_SET_NONE,
	ANIM_SET_PLAYER1,
};


// AnimationId：
 // 资源层动画 ID。Animator 选择 AnimationState 后，
 // 会通过 getPlayerAnimationId 转换成 AnimationId，再向 ResourceManager 请求 AnimationClip。
enum AnimationId
{
	ANIM_ID_PLAYER_IDLE_L,
	ANIM_ID_PLAYER_IDLE_R,
	ANIM_ID_PLAYER_WALK_L,
	ANIM_ID_PLAYER_WALK_R,
	ANIM_ID_PLAYER_RUN_L,
	ANIM_ID_PLAYER_RUN_R,
	ANIM_ID_PLAYER_JUMP_START_L,
	ANIM_ID_PLAYER_JUMP_START_R,
	ANIM_ID_PLAYER_JUMP_LOOP_L,
	ANIM_ID_PLAYER_JUMP_LOOP_R,
	ANIM_ID_PLAYER_JUMP_END_L,
	ANIM_ID_PLAYER_JUMP_END_R,
	ANIM_ID_COUNT
};

// AnimationClip：
 // 动画资源描述数据。
 // 它描述“哪张图、多少帧、播放速度、是否循环”，也描述 sprite sheet 的裁剪规则。
 // 它不保存当前播放进度，当前播放进度由 animatedSprite / 未来的 AnimationPlayer 负责。
struct AnimationClip
{
	IMAGE* image;
	int frameCount;
	int speed;
	bool loop;

    // 单帧宽高，为 0 时由动画播放器按旧规则自动计算。
    int frameWidth;
    int frameHeight;

    // 当前 clip 在 sprite sheet 中的起始裁剪坐标。
    int sourceStartX;
    int sourceStartY;

    // 相邻帧之间的横向和纵向间隔。
    int frameSpacingX;
    int frameSpacingY;

    // 每行帧数，为 0 时暂时按横向单行动画处理。
    int frameColumns;



	// 功能：初始化一个空动画片段描述。
    AnimationClip()
    {
        image = NULL;
        frameCount = 1;
        speed = 4;
        loop = true;

        frameWidth = 0;
        frameHeight = 0;

        sourceStartX = 0;
        sourceStartY = 0;

        frameSpacingX = 0;
        frameSpacingY = 0;

        frameColumns = 0;
    }

	// 功能：按图片指针、帧数、速度和循环标记创建动画片段描述。
    AnimationClip(IMAGE* newImage, int newFrameCount, int newSpeed, bool newLoop)
    {
        image = newImage;
        frameCount = newFrameCount;
        speed = newSpeed;
        loop = newLoop;

        frameWidth = 0;
        frameHeight = 0;

        sourceStartX = 0;
        sourceStartY = 0;

        frameSpacingX = 0;
        frameSpacingY = 0;

        frameColumns = 0;
    }

    // 功能：按完整 sprite sheet 裁剪配置创建动画片段描述。
    AnimationClip(
        IMAGE* newImage,
        int newFrameCount,
        int newSpeed,
        bool newLoop,
        int newFrameWidth,
        int newFrameHeight,
        int newSourceStartX,
        int newSourceStartY,
        int newFrameSpacingX,
        int newFrameSpacingY,
        int newFrameColumns
    )
    {
        image = newImage;
        frameCount = newFrameCount;
        speed = newSpeed;
        loop = newLoop;

        frameWidth = newFrameWidth;
        frameHeight = newFrameHeight;

        sourceStartX = newSourceStartX;
        sourceStartY = newSourceStartY;

        frameSpacingX = newFrameSpacingX;
        frameSpacingY = newFrameSpacingY;

        frameColumns = newFrameColumns;
    }

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


// 功能：注册项目内置字体，供 EasyX 文本绘制使用。
void loadUIFont()
{
    AddFontResourceEx(
        _T("Mojangles.ttf"),
        FR_PRIVATE,
        NULL
    );
}


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
// 功能：把动画表现状态转换为玩家动画资源 ID。
AnimationId getPlayerAnimationId(AnimationState state)
{
	if (state == ANIM_IDLE_L)
	{
		return ANIM_ID_PLAYER_IDLE_L;
	}

	if (state == ANIM_IDLE_R)
	{
		return ANIM_ID_PLAYER_IDLE_R;
	}

	if (state == ANIM_WALK_LEFT)
	{
		return ANIM_ID_PLAYER_WALK_L;
	}

	if (state == ANIM_WALK_RIGHT)
	{
		return ANIM_ID_PLAYER_WALK_R;
	}

	if (state == ANIM_RUN_LEFT)
	{
		return ANIM_ID_PLAYER_RUN_L;
	}

	if (state == ANIM_RUN_RIGHT)
	{
		return ANIM_ID_PLAYER_RUN_R;
	}
	if (state == ANIM_JUMP_START_L)
	{
		return ANIM_ID_PLAYER_JUMP_START_L;
	}

	if (state == ANIM_JUMP_START_R)
	{
		return ANIM_ID_PLAYER_JUMP_START_R;
	}

	if (state == ANIM_JUMP_LOOP_L)
	{
		return ANIM_ID_PLAYER_JUMP_LOOP_L;
	}

	if (state == ANIM_JUMP_LOOP_R)
	{
		return ANIM_ID_PLAYER_JUMP_LOOP_R;
	}

	if (state == ANIM_JUMP_END_L)
	{
		return ANIM_ID_PLAYER_JUMP_END_L;
	}

	if (state == ANIM_JUMP_END_R)
	{
		return ANIM_ID_PLAYER_JUMP_END_R;
	}
	return ANIM_ID_PLAYER_IDLE_L;
}

// 功能：根据动画资源组和动画表现状态，解析出资源层 AnimationId。
AnimationId getAnimationId(AnimationSetId setId, AnimationState state)
{
    if (setId == ANIM_SET_PLAYER1)
    {
        return getPlayerAnimationId(state);
    }

    return ANIM_ID_COUNT;
}

// ResourceManager：
 // 当前关卡资源管理器。
 // 目前主要负责提前加载玩家动画 IMAGE，并根据 AnimationId 返回 AnimationClip。
 // 后续可以继续接管地图、音效、其它实体动画等资源。
class ResourceManager
{
private:
	IMAGE playerIdleL;
	IMAGE playerIdleR;
	IMAGE playerWalkL;
	IMAGE playerWalkR;
	IMAGE playerRunL;
	IMAGE playerRunR;
	IMAGE playerJumpStartL;
	IMAGE playerJumpStartR;
	IMAGE playerJumpLoopL;
	IMAGE playerJumpLoopR;
	IMAGE playerJumpEndL;
	IMAGE playerJumpEndR;

public:
	// 功能：加载当前关卡需要的动画图片资源。
	void loadLevelResources()
	{
		loadimage(&playerIdleL, _T("assets\\tex\\entities\\characters\\player1_idle_L.png"));
		loadimage(&playerIdleR, _T("assets\\tex\\entities\\characters\\player1_idle_R.png"));
		loadimage(&playerWalkL, _T("assets\\tex\\entities\\characters\\player1_walk_L.png"));
		loadimage(&playerWalkR, _T("assets\\tex\\entities\\characters\\player1_walk_R.png"));
		loadimage(&playerRunL, _T("assets\\tex\\entities\\characters\\player1_run_L.png"));
		loadimage(&playerRunR, _T("assets\\tex\\entities\\characters\\player1_run_R.png"));
		loadimage(&playerJumpStartL, _T("assets\\tex\\entities\\characters\\player1_jumpStart_L.png"));
		loadimage(&playerJumpStartR, _T("assets\\tex\\entities\\characters\\player1_jumpStart_R.png"));
		loadimage(&playerJumpLoopL, _T("assets\\tex\\entities\\characters\\player1_jumpLoop_L.png"));
		loadimage(&playerJumpLoopR, _T("assets\\tex\\entities\\characters\\player1_jumpLoop_R.png"));
		loadimage(&playerJumpEndL, _T("assets\\tex\\entities\\characters\\player1_jumpEnd_L.png"));
		loadimage(&playerJumpEndR, _T("assets\\tex\\entities\\characters\\player1_jumpEnd_R.png"));
	}

	// 功能：根据动画 ID 获取对应动画资源描述。
	AnimationClip getAnimationClip(AnimationId id)
	{
		if (id == ANIM_ID_PLAYER_IDLE_L)
		{
			return AnimationClip(&playerIdleL, 8, 3, true);
		}

		if (id == ANIM_ID_PLAYER_IDLE_R)
		{
			return AnimationClip(&playerIdleR, 8, 3, true);
		}

		if (id == ANIM_ID_PLAYER_WALK_L)
		{
			return AnimationClip(&playerWalkL, 8, 3, true);
		}

		if (id == ANIM_ID_PLAYER_WALK_R)
		{
			return AnimationClip(&playerWalkR, 8, 3, true);
		}

		if (id == ANIM_ID_PLAYER_RUN_L)
		{
			return AnimationClip(&playerRunL, 8, 3, true);
		}

		if (id == ANIM_ID_PLAYER_RUN_R)
		{
			return AnimationClip(&playerRunR, 8, 3, true);
		}
		if (id == ANIM_ID_PLAYER_JUMP_START_L)
		{
			return AnimationClip(&playerJumpStartL, 8, 2, false);
		}

		if (id == ANIM_ID_PLAYER_JUMP_START_R)
		{
			return AnimationClip(&playerJumpStartR, 8, 2, false);
		}

		if (id == ANIM_ID_PLAYER_JUMP_LOOP_L)
		{
			return AnimationClip(&playerJumpLoopL, 8, 3, true);
		}

		if (id == ANIM_ID_PLAYER_JUMP_LOOP_R)
		{
			return AnimationClip(&playerJumpLoopR, 8, 3, true);
		}

		if (id == ANIM_ID_PLAYER_JUMP_END_L)
		{
			return AnimationClip(&playerJumpEndL, 8, 2, false);
		}

		if (id == ANIM_ID_PLAYER_JUMP_END_R)
		{
			return AnimationClip(&playerJumpEndR, 8, 2, false);
		}
		return AnimationClip();
	}
};


// sprite：
 // 单帧渲染数据容器。
 // 它只记录当前要绘制的图片来源、源图裁剪矩形、可见性、缩放和偏移。
 // 它不负责动画播放、不负责动画状态切换，也不直接调用 EasyX 绘制函数。
struct sprite
{
    IMAGE* imageSource;

    int srcX;
    int srcY;
	int srcW;
	int srcH;

    bool visible;

	double scaleX;
	double scaleY;
	double offsetX;
	double offsetY;

	double worldCenterX;
	double worldCenterY;

    double worldDrawW;
	double worldDrawH;

	// 功能：初始化一个空精灵，默认没有图、帧矩形和变换。
    sprite() : imageSource(NULL),
    srcX(0),
    srcY(0),
    srcW(0),
    srcH(0),
    scaleX(1.0),
    scaleY(1.0),
    offsetX(0.0),
    offsetY(0.0),
    worldCenterX(0.0),
    worldCenterY(0.0),
    worldDrawW(0.0),
    worldDrawH(0.0),
    visible(true)
	{
	}
	// 功能：设置精灵当前帧使用的图像资源和源图裁剪矩形。
    void setSource(IMAGE* newImageSource,int newSrcX, int newSrcY, int newSrcW, int newSrcH)
    {
        imageSource = newImageSource;
        srcX = newSrcX;
        srcY = newSrcY;
        srcW = newSrcW;
        srcH = newSrcH;
    }

	// 功能：设置精灵的缩放和相对实体中心的偏移。
	void setTransform(double newScaleX, double newScaleY, double newOffsetX, double newOffsetY)
	{
		scaleX = newScaleX;
		scaleY = newScaleY;
		offsetX = newOffsetX;
		offsetY = newOffsetY;
	}

    // 功能：设置 sprite 在世界坐标中的最终绘制中心点和绘制尺寸。
    void setWorldDrawData(double newCenterX, double newCenterY, double newDrawW, double newDrawH)
    {
        worldCenterX = newCenterX;
        worldCenterY = newCenterY;
        worldDrawW = newDrawW;
        worldDrawH = newDrawH;
    }
};


// BackgroundLayer：
// 单个背景层的数据对象。
// 它保存图片、视差系数、缩放响应系数和绘制开关。
struct BackgroundLayer
{
    IMAGE image;

    int renderOrder;

    double parallaxFactor;
    double zoomFactor;

    bool visible;
    bool repeatX;
    bool useAlphaBlend;

    // 功能：初始化背景层默认数据。
    BackgroundLayer()
    {
        renderOrder = 0;

        parallaxFactor = 0.0;
        zoomFactor = 0.0;

        visible = true;
        repeatX = true;
        useAlphaBlend = true;
    }

    // 功能：加载背景层图片并设置基础绘制参数。
    void load(
        const TCHAR* imagePath,
        int newRenderOrder,
        double newParallaxFactor,
        double newZoomFactor,
        bool newUseAlphaBlend
    )
    {
        loadimage(&image, imagePath, WINDOW_WIDTH, WINDOW_HEIGHT, true);

        renderOrder = newRenderOrder;
        parallaxFactor = newParallaxFactor;
        zoomFactor = newZoomFactor;
        useAlphaBlend = newUseAlphaBlend;
    }
};



// BackgroundManager：
// 管理当前关卡中的所有背景层。
// 当前只负责保存背景层数据，后续会负责按相机状态生成 background sprite。
class BackgroundManager
{
private:
    vector<BackgroundLayer> layers;

public:
    // 功能：清空所有背景层。
    void clear()
    {
        layers.clear();
    }

    // 功能：添加一个背景层。
    void addLayer(const BackgroundLayer& layer)
    {
        layers.push_back(layer);
    }

    // 功能：获取背景层数量。
    int getLayerCount() const
    {
        return (int)layers.size();
    }

    // 功能：获取背景层只读列表。
    const vector<BackgroundLayer>& getLayers() const
    {
        return layers;
    }
};


// animatedSprite：
 // 过渡期的序列帧动画播放器，后续可以进一步改名为 AnimationPlayer。
 // 它根据 AnimationClip 绑定图片资源，推进 currentFrame，并把当前帧写入 sprite。
 // 注意：它不再负责绘制，不负责实体输入、移动、碰撞，也不负责动画状态机判断。
class animatedSprite
{
private:
	IMAGE image;
	IMAGE* imageSource;

    int frameCount;
    int currentFrame;

    int frameWidth;
    int frameHeight;

    int sourceStartX;
    int sourceStartY;

    int frameSpacingX;
    int frameSpacingY;

    int frameColumns;

    bool isPlaying;
    bool isLoop;

    int frameInterval;//帧间隔
    int frameTimer;//计时器

public:
    // 功能：初始化序列帧动画的默认播放参数。
    animatedSprite()//基础构造
    {
        frameWidth = 0;
        frameHeight = 0;

        sourceStartX = 0;
        sourceStartY = 0;

        frameSpacingX = 0;
        frameSpacingY = 0;

        frameColumns = 0;

        frameCount = 0;
        currentFrame = 0;

        frameInterval = 8;
        frameTimer = 0;

        isPlaying = true;
        isLoop = true;

        imageSource = &image;
    }

    //接口，判定是否播放结束
	bool isFinished()
	{
		return !isLoop && !isPlaying;
	}
    // 功能：按显式帧尺寸加载序列帧图片。
    void load(const TCHAR* path, int frameWidth, int frameHeight, int frameCount)
    {
        loadimage(&image, path);
        imageSource = &image;
        this->frameWidth = frameWidth;
        this->frameHeight = frameHeight;
        this->frameCount = frameCount;

        sourceStartX = 0;
        sourceStartY = 0;

        frameSpacingX = 0;
        frameSpacingY = 0;

        frameColumns = frameCount;

        currentFrame = 0;
        frameTimer = 0;
        isPlaying = 1;

    }
    // 功能：按帧数自动平均切分横向序列帧图片。
    void load(const TCHAR* path, int newFrameCount)
    {
        loadimage(&image, path);
		imageSource = &image;

        frameCount = newFrameCount;

        if (frameCount < 1)
        {
            frameCount = 1;
        }

        frameWidth = image.getwidth() / frameCount;
        frameHeight = image.getheight();

        sourceStartX = 0;
        sourceStartY = 0;

        frameSpacingX = 0;
        frameSpacingY = 0;

        frameColumns = frameCount;

        currentFrame = 0;
        frameTimer = 0;
        isPlaying = true;
    }

	// 功能：绑定已经由 ResourceManager 加载好的动画资源。
	void setClip(AnimationClip clip)
	{
		if (clip.image == NULL)
		{
			return;
		}

		imageSource = clip.image;

		frameCount = clip.frameCount;

		if (frameCount < 1)
		{
			frameCount = 1;
		}
		// 如果动画片段描述里指定了单帧宽高，就用它；否则按旧规则平均切分。
        if (clip.frameWidth > 0)
        {
            frameWidth = clip.frameWidth;
        }
        else
        {
            // 用图片总宽度除以帧数，得到旧横向单行动画的单帧宽度。
            frameWidth = imageSource->getwidth() / frameCount;
        }
		// 同上，指定了单帧高度就用它，否则默认整张图高就是单帧高。
        if (clip.frameHeight > 0)
        {
            frameHeight = clip.frameHeight;
        }
        else
        {
            // 用图片总高度作为单帧高度，兼容旧横向单行动画。
            frameHeight = imageSource->getheight();
        }

        sourceStartX = clip.sourceStartX;
        sourceStartY = clip.sourceStartY;

        frameSpacingX = clip.frameSpacingX;
        frameSpacingY = clip.frameSpacingY;

        if (clip.frameColumns > 0)
        {
            frameColumns = clip.frameColumns;
        }
        else
        {
            // 未显式指定列数时，用总帧数作为列数，等价于旧的单行动画。
            frameColumns = frameCount;
        }

		setSpeed(clip.speed);
		setLoop(clip.loop);

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
            // 每到达帧间隔就推进一帧，currentFrame 表示当前播放到第几帧。
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

	//功能：将当前计算出的动画帧的数据写入一个基础 sprite 结构中，供 Entity 绘制使用。
    void writeCurrentFrameTo(sprite& targetSprite)
    {
        if (imageSource == NULL)
        {
            targetSprite.setSource(NULL, 0, 0, 0, 0);
            return;
        }
        if(frameCount <= 0||frameWidth <= 0||frameHeight <= 0)
        {
            targetSprite.setSource(NULL, 0, 0, 0, 0);
            return;
        }

        int activeColumns = frameColumns;

        if (activeColumns < 1)
        {
            activeColumns = frameCount;
        }

        if (activeColumns < 1)
        {
            activeColumns = 1;
        }

        int frameCol = currentFrame % activeColumns;
        int frameRow = currentFrame / activeColumns;

        // 用当前帧序号换算出行列，再按起点、单帧尺寸和间距计算源图裁剪坐标。
        int srcX = sourceStartX + frameCol * (frameWidth + frameSpacingX);
        int srcY = sourceStartY + frameRow * (frameHeight + frameSpacingY);


		targetSprite.setSource(
            imageSource,
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

	// 功能：初始化碰撞盒的原始尺寸、偏移和缩放。
	CollisionBox()
	{
		width = 0;
		height = 0;
		offsetX = 0.0;
		offsetY = 0.0;
		scaleX = 1.0;
		scaleY = 1.0;
	}

	// 功能：设置碰撞盒的原始尺寸。
	void setBaseSize(double newWidth, double newHeight)
	{
		width = newWidth;
		height = newHeight;
	}

	// 功能：设置碰撞盒相对拥有者中心点的偏移。
	void setOffset(double newOffsetX, double newOffsetY)
	{
		offsetX = newOffsetX;
		offsetY = newOffsetY;
	}

	// 功能：设置碰撞盒的缩放比例。
	void setScale(double newScaleX, double newScaleY)
	{
		scaleX = newScaleX;
		scaleY = newScaleY;
	}

	// 功能：根据拥有者世界坐标生成最终用于检测的世界碰撞盒。
	RectBox toWorldBox(double ownerX, double ownerY)
	{
		RectBox box;

        // 用实体中心点加碰撞盒偏移，得到碰撞盒自己的世界中心点。
		double colliderCenterX = ownerX + offsetX;
		double colliderCenterY = ownerY + offsetY;

        // 用原始宽高乘缩放，得到最终参与检测的世界宽高。
		double colliderWidth = width * scaleX;
		double colliderHeight = height * scaleY;

        // 用中心点加减半宽半高，得到 AABB 的四条世界边界。
		box.left = colliderCenterX - colliderWidth / 2.0;
		box.right = colliderCenterX + colliderWidth / 2.0;
		box.bottom = colliderCenterY - colliderHeight / 2.0;
		box.top = colliderCenterY + colliderHeight / 2.0;

		return box;
	}
};

// TileId：
 // 地图 tile 编号。当前 TILE_EMPTY=0 表示空格，不绘制，也不生成默认地图碰撞。
enum TileId
{
    TILE_EMPTY = 0
};

// TileCollisionType：
// 定义地图碰撞层中每个格子的碰撞规则类型。
enum TileCollisionType
{
    TILE_COLLISION_NONE = 0,                // 不参与地图碰撞。
    TILE_COLLISION_FULL_SOLID = 1,          // 完整 tile 实体碰撞，阻挡上下左右。
    TILE_COLLISION_FULL_ONE_WAY = 2,        // 完整 tile 单向平台，只阻挡从上方下落。
    TILE_COLLISION_TOP_HALF_ONE_WAY = 3,    // 顶部半格单向平台。
    TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY = 4,   // 左上半格单向平台。
    TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY = 5   // 右上半格单向平台。
};


// TileInstance：
// 地图中一个实际摆放出来的 tile 实例。
// tileId 表示它引用 tileset 中哪个图块；row/col 和 world 坐标表示它被摆在哪里。
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

    bool visible;

    TileInstance()
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

        visible = true;
    }
};


// TileMap：
 // 负责加载 tileset 和地图文本数据，并维护地图中的 tile 实例与碰撞层数据。
 // 实际绘制由 Renderer 读取 TileInstance 后统一完成，TileMap 不再直接调用 EasyX 绘图函数。
 // 后续可以扩展为：地图触发器、地图层级、视口剔除、编辑器放置数据等。
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

    // 视觉 tile 二维数组，存放用于绘制的 tile id。
    int** tiles;

    // 碰撞 tile 二维数组，存放 TileCollisionType 对应的编号。
    int** collisionTiles;

    // 地图上所有非空 tile 的实例列表。
    // 当前仍由二维数组生成，后续可以扩展为编辑器直接维护的场景元素列表。
    vector<TileInstance> tileInstances;

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
        collisionTiles = NULL;

        offsetX = 0;
        offsetY = 0;
    }

    // 功能：释放 tile map 动态分配的地图数据。
    ~TileMap()
    {
        release();
    }


	int getRows()
	{
		return rows;
	}

	int getCols()
	{
		return cols;
	}


    // 功能：获取当前地图中的 tile 实例数量。
    int getTileInstanceCount() const
    {
        return (int)tileInstances.size();
    }

    // 功能：获取指定下标的 tile 实例。
    const TileInstance& getTileInstance(int index) const
    {
        return tileInstances[index];
    }

    // 功能：获取当前地图的 tile 实例列表。
    const vector<TileInstance>& getTileInstances() const
    {
        return tileInstances;
    }


    // 功能：设置指定 tile 实例的绘制偏移。
    void setTileInstanceOffset(int index, double newOffsetX, double newOffsetY)
    {
        if (index < 0 || index >= (int)tileInstances.size())
        {
            return;
        }

        tileInstances[index].offsetX = newOffsetX;
        tileInstances[index].offsetY = newOffsetY;
    }

    // 功能：设置指定 tile 实例的绘制缩放。
    void setTileInstanceScale(int index, double newScaleX, double newScaleY)
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
    void setTileInstanceTransform(
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
    int findTileInstanceIndexByGrid(int targetRow, int targetCol) const
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
    // 如果目标格子是空 tile，则不会存在对应实例，函数会直接跳过。
    void setTileInstanceTransformByGrid(
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
     sprite buildSpriteFromTileInstance(const TileInstance& tile)
    {
		sprite tileSprite;

		tileSprite.visible = tile.visible;

        if(!tile.visible)
        {
            return tileSprite;
        }

        if(tile.tileId==TILE_EMPTY)
        {
			tileSprite.visible = false;
            return tileSprite;
        }

        if (sourceTileWidth <= 0 || sourceTileHeight <= 0)
        {
            tileSprite.visible = false;
            return tileSprite;
        }

		int tilesetCols = tileset.getwidth() / sourceTileWidth;

        if(tilesetCols <= 0)
        {
            tileSprite.visible = false;
            return tileSprite;
        }

		int realTileIndex = tile.tileId - 1;

        //用tileId在tileset中的线性序号换算出原图的裁剪坐标

        int srcX = (realTileIndex % tilesetCols) * sourceTileWidth;
        int srcY = (realTileIndex / tilesetCols) * sourceTileHeight;

        // 设置 sprite 的纹理坐标等信息
		tileSprite.setSource(
			&tileset,
			srcX,
			srcY,
			sourceTileWidth,
			sourceTileHeight
		);

		//用默认tile世界尺寸乘以实例缩放，得到本帧最终绘制尺寸
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
    void releaseCollisionTiles()
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
        // 视觉层重载时同步释放碰撞层，避免新旧地图尺寸不一致。
        releaseCollisionTiles();

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

    // 功能：根据视觉 tile id 返回默认碰撞类型。
    TileCollisionType getDefaultCollisionTypeByTileId(int tileId)
    {
        if (tileId == TILE_EMPTY)
        {
            return TILE_COLLISION_NONE;
        }

        // 这些 tile 暂时作为完整固体处理，用于箱子、砖块、金属块等硬阻挡。
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

        // 草地平台主体默认是顶部半格单向平台：下落时站住，向上跳时穿过。
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
    void generateDefaultCollisionFromTiles()
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
    void rebuildTileInstances()
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

                // 先用地图偏移、列号和反向行号，计算这个 tile 默认所在网格的世界左下角。
                // 当前 drawTileWidth / drawTileHeight 暂时同时承担“网格单元大小”和“默认绘制大小”。
                double gridWorldLeft = offsetX + col * drawTileWidth;
                double gridWorldBottom = offsetY + (rows - 1 - row) * drawTileHeight;

                // 用网格左下角加半个单元格，得到 tile 实例的默认世界中心点。
                // 后续 Renderer 会以 centerX / centerY 为锚点计算最终绘制矩形。
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

        generateDefaultCollisionFromTiles();

		rebuildTileInstances();


        return true;
    }

    // 功能：获取指定行列的 tile 碰撞类型。
    TileCollisionType getTileCollisionType(int row, int col)
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
    bool hasTileCollision(int row, int col)
    {
        return getTileCollisionType(row, col) != TILE_COLLISION_NONE;
    }

    // 功能：获取指定 tile 在世界坐标中的矩形范围。
    RectBox getTileWorldBox(int row, int col)
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
    RectBox getTileCollisionWorldBox(int row, int col)
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
    void drawDebugCollisionBoxes()
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

// 功能：获取当前窗口对应的顶层 UI 区域。
UIBox makeViewportUIBox()
{
    UIBox box;

    box.x = 0;
    box.y = 0;
    box.w = WINDOW_WIDTH;
    box.h = WINDOW_HEIGHT;

    return box;
}


// 功能：根据父级 UIBox、锚点和 margin 计算子 UI 的屏幕位置。
UIBox makeUIBoxByParentAnchor(
    UIBox parent,
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
        box.x = parent.x + marginX;
        box.y = parent.y + marginY;
    }
    else if (anchor == UI_TOP_RIGHT)
    {
        box.x = parent.x + parent.w - w - marginX;
        box.y = parent.y + marginY;
    }
    else if (anchor == UI_BOTTOM_LEFT)
    {
        box.x = parent.x + marginX;
        box.y = parent.y + parent.h - h - marginY;
    }
    else if (anchor == UI_BOTTOM_RIGHT)
    {
        box.x = parent.x + parent.w - w - marginX;
        box.y = parent.y + parent.h - h - marginY;
    }
    else
    {
        box.x = parent.x + parent.w / 2 - w / 2 + marginX;
        box.y = parent.y + parent.h / 2 - h / 2 + marginY;
    }

    return box;
}



// 功能：根据窗口锚点和 margin 计算顶层 UI 的屏幕位置。
UIBox makeUIBoxByAnchor(
    int w,
    int h,
    UIAnchor anchor,
    int marginX,
    int marginY
)
{
    return makeUIBoxByParentAnchor(
        makeViewportUIBox(),
        w,
        h,
        anchor,
        marginX,
        marginY
    );
}

// UIElementState：
// UI 元素生命周期状态。用于区分隐藏、进入、显示和退出。
enum UIElementState
{
    UI_HIDDEN,
    UI_SHOWING,
    UI_VISIBLE,
    UI_HIDING
};

// UIElement：
// 通用 UI 元素基础类。
// 它只使用屏幕坐标 / 父级 UI 坐标，不依赖世界坐标和 Camera。
class UIElement
{
private:
    double x;
    double y;

    double targetX;
    double targetY;

    int w;
    int h;

    UIAnchor anchor;
    int marginX;
    int marginY;

    int parentIndex;

    bool active;
    bool visible;
    bool interactable;

    double moveSpeed;

    UIElementState state;

public:
    // 功能：初始化一个默认隐藏的 UI 元素。
    UIElement()
    {
        x = 0;
        y = 0;

        targetX = 0;
        targetY = 0;

        w = 0;
        h = 0;

        anchor = UI_TOP_LEFT;
        marginX = 0;
        marginY = 0;

        parentIndex = -1;

        active = false;
        visible = false;
        interactable = false;

        moveSpeed = 0.2;

        state = UI_HIDDEN;
    }

    // 功能：立即把当前位置同步到目标位置。
    void snapToTarget()
    {
        x = targetX;
        y = targetY;
    }

    // 功能：设置当前 UI 元素的父级元素下标，-1 表示父级为窗口。
    void setParentIndex(int newParentIndex)
    {
        parentIndex = newParentIndex;
    }

    // 功能：获取当前 UI 元素的父级元素下标。
    int getParentIndex() const
    {
        return parentIndex;
    }


    // 功能：直接设置 UI 元素当前位置。
    void setPosition(double newX, double newY)
    {
        x = newX;
        y = newY;
    }

    // 功能：直接设置 UI 元素目标位置。
    void setTargetPosition(double newTargetX, double newTargetY)
    {
        targetX = newTargetX;
        targetY = newTargetY;
    }

    // 功能：获取 UI 元素当前目标 X。
    double getTargetX() const
    {
        return targetX;
    }

    // 功能：获取 UI 元素当前目标 Y。
    double getTargetY() const
    {
        return targetY;
    }

    // 功能：按窗口锚点初始化 UI 元素，并默认显示在目标位置。
    void init(int newW, int newH, UIAnchor newAnchor, int newMarginX, int newMarginY)
    {
        w = newW;
        h = newH;

        anchor = newAnchor;
        marginX = newMarginX;
        marginY = newMarginY;

        parentIndex = -1;

        refreshTarget();

        x = targetX;
        y = targetY;

        active = true;
        visible = true;
        interactable = true;

        state = UI_VISIBLE;
    }
    // 功能：按窗口作为父级重新计算目标位置。
    void refreshTarget()
    {
        refreshTargetByParentBox(makeViewportUIBox());
    }
    // 功能：切换 UI 锚点，并把新锚点位置作为移动目标。
    void setAnchor(UIAnchor newAnchor)
    {
        anchor = newAnchor;
        refreshTarget();
    }

    // 功能：根据指定父级 UIBox 重新计算目标位置。
    void refreshTargetByParentBox(UIBox parentBox)
    {
        UIBox box = makeUIBoxByParentAnchor(
            parentBox,
            w,
            h,
            anchor,
            marginX,
            marginY
        );

        targetX = box.x;
        targetY = box.y;
    }


    // 功能：立即显示 UI 元素，不播放进入动画。
    void showInstant()
    {
        active = true;
        visible = true;
        interactable = true;

        state = UI_VISIBLE;
    }

    // 功能：显示 UI 元素，并进入过渡动画状态。
    void showAnimated()
    {
        active = true;
        visible = true;
        interactable = false;

        state = UI_SHOWING;
    }

    // 功能：兼容旧调用，默认使用动画显示；目标位置由 UIManager 按父级关系刷新。
    void show()
    {
        showAnimated();
    }

    // 功能：立即隐藏 UI 元素，不播放退出动画。
    void hide()
    {
        active = false;
        visible = false;
        interactable = false;

        state = UI_HIDDEN;
    }

    // 功能：进入隐藏动画状态，动画结束后才真正隐藏。
    void hideAnimated()
    {
        active = true;
        visible = true;
        interactable = false;

        state = UI_HIDING;
    }

    // 功能：平滑推进 UI 元素当前位置，使其靠近目标位置。
    void update()
    {
        if (!active)
        {
            return;
        }

        x += (targetX - x) * moveSpeed;
        y += (targetY - y) * moveSpeed;

        if (fabs(targetX - x) < 0.1)
        {
            x = targetX;
        }

        if (fabs(targetY - y) < 0.1)
        {
            y = targetY;
        }

        if (state == UI_SHOWING && x == targetX && y == targetY)
        {
            interactable = true;
            state = UI_VISIBLE;
        }

        if (state == UI_HIDING && x == targetX && y == targetY)
        {
            active = false;
            visible = false;
            interactable = false;
            state = UI_HIDDEN;
        }
    }

    // 功能：获取 UI 元素当前屏幕矩形。
    UIBox getBox() const
    {
        UIBox box;

        box.x = (int)x;
        box.y = (int)y;
        box.w = w;
        box.h = h;

        return box;
    }

    // 功能：判断 UI 元素是否参与 update。
    bool isActive() const
    {
        return active;
    }

    // 功能：判断 UI 元素是否参与 render。
    bool isVisible() const
    {
        return visible;
    }

    // 功能：判断 UI 元素是否允许交互。
    bool isInteractable() const
    {
        return interactable;
    }

    // 功能：获取 UI 元素当前生命周期状态。
    UIElementState getState() const
    {
        return state;
    }
};


// UIManager：
// 管理当前界面中的 UIElement。
// 第一版只负责保存元素、根据父级关系刷新位置、统一 update，不处理自动布局。
class UIManager
{
private:
    vector<UIElement> elements;

public:
    // 功能：添加一个 UI 元素，返回它在 UIManager 中的下标。
    int addElement(UIElement element)
    {
        elements.push_back(element);
        return (int)elements.size() - 1;
    }

    // 功能：判断 UI 元素下标是否有效。
    bool isValidIndex(int index) const
    {
        return index >= 0 && index < (int)elements.size();
    }

    // 功能：判断 UI 元素在父级链路影响下最终是否可见。
    bool isElementEffectivelyVisible(int index) const
    {
        if (!isValidIndex(index))
        {
            return false;
        }

        const UIElement& element = elements[index];

        if (!element.isVisible())
        {
            return false;
        }

        int parentIndex = element.getParentIndex();

        if (parentIndex < 0)
        {
            return true;
        }

        return isElementEffectivelyVisible(parentIndex);
    }

    // 功能：获取指定 UI 元素的可写引用。
    UIElement& getElement(int index)
    {
        return elements[index];
    }

    // 功能：获取指定 UI 元素的只读引用。
    const UIElement& getElement(int index) const
    {
        return elements[index];
    }

    // 功能：获取指定 UI 元素当前屏幕矩形。
    UIBox getElementBox(int index) const
    {
        if (!isValidIndex(index))
        {
            return makeViewportUIBox();
        }

        return elements[index].getBox();
    }

    // 功能：根据父级关系刷新指定 UI 元素的目标位置。
    void refreshElementTarget(int index)
    {
        if (!isValidIndex(index))
        {
            return;
        }

        int parentIndex = elements[index].getParentIndex();

        UIBox parentBox;

        if (isValidIndex(parentIndex))
        {
            parentBox = elements[parentIndex].getBox();
        }
        else
        {
            parentBox = makeViewportUIBox();
        }

        elements[index].refreshTargetByParentBox(parentBox);
    }

    // 功能：按父级关系刷新位置后立即显示指定 UI 元素。
    void showElementInstant(int index)
    {
        if (!isValidIndex(index))
        {
            return;
        }

        refreshElementTarget(index);
        elements[index].showInstant();
        elements[index].snapToTarget();
    }

    // 功能：按父级关系刷新位置后以动画方式显示指定 UI 元素。
    void showElementAnimated(int index)
    {
        if (!isValidIndex(index))
        {
            return;
        }

        refreshElementTarget(index);
        elements[index].showAnimated();
    }

    // 功能：按父级关系刷新目标位置后，从指定偏移位置滑入。
    void showElementAnimatedFromOffset(int index, double offsetX, double offsetY)
    {
        if (!isValidIndex(index))
        {
            return;
        }

        refreshElementTarget(index);

        double targetX = elements[index].getTargetX();
        double targetY = elements[index].getTargetY();

        elements[index].setPosition(targetX + offsetX, targetY + offsetY);
        elements[index].showAnimated();
    }

    // 功能：按父级关系取得正常目标位置，再把目标改为偏移位置并播放隐藏动画。
    void hideElementAnimatedToOffset(int index, double offsetX, double offsetY)
    {
        if (!isValidIndex(index))
        {
            return;
        }

        refreshElementTarget(index);

        double targetX = elements[index].getTargetX();
        double targetY = elements[index].getTargetY();

        elements[index].setTargetPosition(targetX + offsetX, targetY + offsetY);
        elements[index].hideAnimated();
    }

    // 功能：立即隐藏指定 UI 元素。
    void hideElementInstant(int index)
    {
        if (!isValidIndex(index))
        {
            return;
        }

        elements[index].hide();
    }

    // 功能：统一更新所有 UI 元素的位置和状态。
    void update()
    {
        for (int i = 0; i < (int)elements.size(); i++)
        {
            int parentIndex = elements[i].getParentIndex();

            UIBox parentBox;

            if (isValidIndex(parentIndex))
            {
                parentBox = elements[parentIndex].getBox();
            }
            else
            {
                parentBox = makeViewportUIBox();
            }

            // UI_HIDING 使用 hideElementAnimatedToOffset() 指定的离场目标，
            // 不能每帧刷新回父级位置，否则隐藏动画会被覆盖。
            if (elements[i].getState() != UI_HIDING)
            {
                elements[i].refreshTargetByParentBox(parentBox);
            }

            elements[i].update();
        }
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

// 前置声明：
// Animator、CollisionHandle、MovementHandle 都需要引用 Entity，
// 而 Entity 内部又持有 Animator，因此这里先声明 Entity。
class Entity;

// Animator：
 // 动画状态控制组件。
 // 它被 Entity 持有，但不拥有实体真实游戏状态。
 // 它读取 Entity 的移动、跳跃、落地、冲刺、朝向等状态，
 // 决定当前应该播放哪个 AnimationState，并通过 ResourceManager 获取 AnimationClip。
class Animator
{
private:
	// 当前动画表现状态，用于避免每帧重复 setClip。
	AnimationState currentAnimState;

	// 上一帧是否在空中，用于判断“刚刚落地”。
	bool wasInAir;

    // 当前 Animator 绑定的动画资源组，用于把 AnimationState 解析为具体 AnimationClip。
    AnimationSetId animationSetId;

    // 初始化时要绑定的动画表现状态，用于在第一帧绘制前设置默认 clip。
    AnimationState initialAnimState;

public:
	// 功能：初始化 Animator 的默认动画状态缓存。
	Animator();

	// 功能：切换实体当前动画片段，若状态未变化则直接返回。
	void changeAnimation(Entity& entity, AnimationState newState, ResourceManager& resources);

	// 功能：根据实体真实状态和本帧行为意图更新动画表现状态。
    void update(Entity& entity, BehaviorIntent intent, ResourceManager& resources);

    // 功能：配置当前 Animator 使用的动画资源组和初始动画状态。
    void configure(AnimationSetId newSetId, AnimationState newInitialState);

    // 功能：在资源加载完成后，根据初始动画状态为实体绑定第一段动画。
    void initAnimation(Entity& entity, ResourceManager& resources);
};

class CollisionHandle;
class MovementHandle;

// 实体类

// Entity：
 // 实体数据容器 + 组件持有者。
 // 移动/阻挡/边界修正逻辑已经迁移到 MovementHandle 和 CollisionHandle。
 // 动画表现状态切换已经迁移到 Animator，实体绘制已经迁移到 Renderer。
 // 因此 Entity 主要保存真实游戏状态，并向外提供位置、碰撞盒、动画播放器和 sprite 数据接口。
 //
 // Entity 当前主要保存：
 //   - 世界坐标 x/y
 //   - 速度 speed / velocityY
 //   - 状态 onGround / InAir / sprinting / jumping / isAlive
 //   - 碰撞配置 collisionBox
 //   - 动画播放对象 animation
 //   - 当前单帧渲染数据 renderSprite
 //   - 动画控制组件 animator
 //   - 类型 entityType
class Entity
{
	// 声明 Animator、CollisionHandle、MovementHandle 为 Entity 的友元类，使它们可以访问 Entity 的私有状态。
    friend class MovementHandle;
    friend class CollisionHandle;
	friend class Animator;
private:
    animatedSprite animation; // 当前动画播放器，负责推进帧并写入 renderSprite。
	sprite renderSprite;      // 实际用于 Renderer 绘制的单帧 sprite 数据。
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
    facingDirection currentFacingDirection;//记录当前操作的有效朝向
	Animator animator;//实体的动画控制器
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

        collisionBox.setBaseSize(imgW, imgH);

        currentFacingDirection = LEFT;

        entityType = Type;
        isAlive = alive;
    }

    // 功能：按初始逻辑状态和动画资源组创建实体，不再直接从构造函数加载图片路径。
    Entity(
        double startX,
        double startY,
        bool isControlled,
        bool isCollidable,
        bool isBlocking,
        bool isGod,
        EntityType Type,
        AnimationSetId animationSet,
        facingDirection initialFacing,
        AnimationState initialAnim,
        bool alive = 1
    )
    {
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

        currentFacingDirection = initialFacing;

        entityType = Type;
        isAlive = alive;

        animator.configure(animationSet, initialAnim);
    }


    // 功能：按默认朝向和默认待机状态创建绑定动画资源组的实体。
    Entity(
        double startX,
        double startY,
        bool isControlled,
        bool isCollidable,
        bool isBlocking,
        bool isGod,
        EntityType Type,
        AnimationSetId animationSet,
        bool alive = 1
    )
        : Entity(
            startX,
            startY,
            isControlled,
            isCollidable,
            isBlocking,
            isGod,
            Type,
            animationSet,
            RIGHT,
            ANIM_IDLE_R,
            alive
        )
    {}

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
	// 功能：判断实体是否由玩家输入控制。
	bool isControlled()
	{
		return controlled;
	}

	// 功能：返回实体当前朝向。
	facingDirection getFacingDirection()
	{
		return currentFacingDirection;
	}
	// 功能：设置实体当前朝向。
	void setFacingDirection(facingDirection direction)
	{
		currentFacingDirection = direction;
	}
	// 功能：判断实体当前非循环动画是否已经播放结束。
	bool isAnimationFinished()
	{
		return animation.isFinished();
	}

	// 功能：把新的动画片段绑定到实体的动画播放器。
	void setAnimationClip(AnimationClip clip)
	{
		animation.setClip(clip);
	}

	// 功能：委托实体内部 Animator 更新动画状态。
	void updateAnimator(BehaviorIntent intent, ResourceManager& resources)
	{
		animator.update(*this, intent, resources);
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

	sprite& getRenderSprite()// 获取实体当前用于渲染的 sprite 可写接口。
	{
		return renderSprite;
	}

	const sprite& getSprite() const// 获取实体当前用于渲染的 sprite 只读接口。
	{
		return renderSprite;
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
		return collisionBox.toWorldBox(testX, testY);

	}

    // 功能：获取实体当前位置下的世界碰撞盒。
    RectBox getWorldCollisionBox()
    {
        return getWorldCollisionBoxAt(x, y);
    }

	// 功能：设置实体碰撞盒原始尺寸。
	void setCollisionBoxSize(double width, double height)
	{
		collisionBox.setBaseSize(width, height);
	}

	// 功能：设置实体碰撞盒相对实体中心点的偏移。
	void setCollisionBoxOffset(double offsetX, double offsetY)
	{
		collisionBox.setOffset(offsetX, offsetY);
	}

    // 功能：设置实体碰撞盒缩放比例。
	void setCollisionScale(double scaleX, double scaleY)
	{
		collisionBox.setScale(scaleX, scaleY);
	}

    // 功能：根据实体当前位置和 sprite 自身变换，补全当前帧的世界绘制数据。
    void syncRenderSpriteWorldDrawData()
    {
        // 用源帧尺寸乘 sprite 缩放，得到当前帧在世界坐标里的绘制宽高。
        double worldDrawW = renderSprite.srcW * renderSprite.scaleX;
        double worldDrawH = renderSprite.srcH * renderSprite.scaleY;

        // 用实体中心点加 sprite 偏移，得到 sprite 本帧的世界中心点。
        double worldCenterX = x + renderSprite.offsetX;
        double worldCenterY = y + renderSprite.offsetY;

        renderSprite.setWorldDrawData(
            worldCenterX,
            worldCenterY,
            worldDrawW,
            worldDrawH
        );
    }

    // 功能：推进实体当前动画播放器，并把当前帧同步写入 renderSprite。
    void updateAnimatedSprite()
    {
        animation.update();
        animation.writeCurrentFrameTo(renderSprite);
        syncRenderSpriteWorldDrawData();
    }


    // 功能：设置实体 sprite 绘制缩放和偏移。
    void setSpriteTransform(double scaleX, double scaleY, double offsetX, double offsetY)
    {
        renderSprite.setTransform(scaleX, scaleY, offsetX, offsetY);
        syncRenderSpriteWorldDrawData();
    }
    // 功能：设置实体动画播放速度。
    void setAnimationSpeed(int speed)
    {
        animation.setSpeed(speed);
    }

    // 功能：让实体内部 Animator 根据初始状态绑定动画，并同步第一帧 sprite 和碰撞盒尺寸。
    void initAnimationFromAnimator(ResourceManager& resources)
    {
        animator.initAnimation(*this, resources);

        animation.writeCurrentFrameTo(renderSprite);
        syncRenderSpriteWorldDrawData();
        if (animation.getFrameWidth() > 0 && animation.getFrameHeight() > 0)
        {
            collisionBox.setBaseSize(
                animation.getFrameWidth(),
                animation.getFrameHeight()
            );
        }
    }
};


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
        vector<Entity>& entitys,
        int selfIndex,
        TileMap& tileMap
    );

    // 功能：声明 Y 轴允许位移计算接口。
    double getAllowedMoveY(
        Entity& self,
        double moveY,
        vector<Entity>& entitys,
        int selfIndex,
        TileMap& tileMap
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
        vector<Entity>& entitys,
        int selfIndex,
        TileMap& tileMap,
        int worldWidth,
        int worldHeight,
        CollisionHandle& collisionHandle
    );
};




// ============================================================
// Animator 实现
// ============================================================

// 功能：初始化动画状态缓存。
Animator::Animator()
{
    currentAnimState = ANIM_COUNT;
    wasInAir = false;
    animationSetId = ANIM_SET_NONE;
    initialAnimState = ANIM_IDLE_R;
}

// 功能：按动画状态切换实体当前播放的 AnimationClip。
void Animator::changeAnimation(Entity& entity, AnimationState newState, ResourceManager& resources)
{
    if (currentAnimState == newState)
    {
        return;
    }

    // 用当前资源组和动画状态解析具体资源 ID，避免 Animator 直接绑定某个角色资源。
    AnimationId animationId = getAnimationId(animationSetId, newState);

    if (animationId == ANIM_ID_COUNT)
    {
        return;
    }

    AnimationClip clip = resources.getAnimationClip(animationId);

    if (clip.image == NULL)
    {
        return;
    }

    // 只有拿到有效 clip 后才写入播放器并更新当前状态缓存。
    entity.setAnimationClip(clip);
    currentAnimState = newState;
}

// 功能：读取实体真实状态并决定 idle / walk / run / jumpStart / jumpLoop / jumpEnd。
void Animator::update(Entity& entity, BehaviorIntent intent, ResourceManager& resources)
{
	if (!entity.isControlled())
	{
		return;
	}

	double inputX = intent.moveX;

	bool hasMoveInput = fabs(inputX) > EPS;
	bool justLanded = wasInAir && entity.isOnGround();



	AnimationState idleState = ANIM_IDLE_L;
	AnimationState walkState = ANIM_WALK_LEFT;
	AnimationState runState = ANIM_RUN_LEFT;
	AnimationState jumpStartState = ANIM_JUMP_START_L;
	AnimationState jumpLoopState = ANIM_JUMP_LOOP_L;
	AnimationState jumpEndState = ANIM_JUMP_END_L;

	if (entity.getFacingDirection() == RIGHT)
	{
		idleState = ANIM_IDLE_R;
		walkState = ANIM_WALK_RIGHT;
		runState = ANIM_RUN_RIGHT;
		jumpStartState = ANIM_JUMP_START_R;
		jumpLoopState = ANIM_JUMP_LOOP_R;
		jumpEndState = ANIM_JUMP_END_R;
	}

	if (justLanded && !hasMoveInput)
	{
		changeAnimation(entity, jumpEndState, resources);
		wasInAir = entity.isInAir();
		return;
	}

	bool currentIsJumpStart =
		currentAnimState == ANIM_JUMP_START_L ||
		currentAnimState == ANIM_JUMP_START_R;

	if (entity.isInAir())
	{
		bool shouldPlayJumpStart =
			entity.isJumping() &&
			intent.wantJump &&
			!wasInAir;

		if (shouldPlayJumpStart)
		{
			changeAnimation(entity, jumpStartState, resources);
			wasInAir = entity.isInAir();
			return;
		}

		if (currentIsJumpStart)
		{
			if (entity.isAnimationFinished())
			{
				changeAnimation(entity, jumpLoopState, resources);
			}

			wasInAir = entity.isInAir();
			return;
		}

		changeAnimation(entity, jumpLoopState, resources);
		wasInAir = entity.isInAir();
		return;
	}

	bool currentIsJumpEnd =
		currentAnimState == ANIM_JUMP_END_L ||
		currentAnimState == ANIM_JUMP_END_R;

	if (currentIsJumpEnd && !hasMoveInput && !entity.isAnimationFinished())
	{
		wasInAir = entity.isInAir();
		return;
	}

	if (hasMoveInput)
	{
		if (entity.isSprinting())
		{
			changeAnimation(entity, runState, resources);
		}
		else
		{
			changeAnimation(entity, walkState, resources);
		}
	}
	else
	{
		changeAnimation(entity, idleState, resources);
	}

	wasInAir = entity.isInAir();
}
// 功能：配置 Animator 的资源组和初始状态，并重置动画状态缓存。
void Animator::configure(AnimationSetId newSetId, AnimationState newInitialState)
{
    animationSetId = newSetId;
    initialAnimState = newInitialState;
    currentAnimState = ANIM_COUNT;
    wasInAir = false;
}

// 功能：在资源加载完成后，根据初始状态绑定实体第一段动画。
void Animator::initAnimation(Entity& entity, ResourceManager& resources)
{
    if (animationSetId == ANIM_SET_NONE)
    {
        return;
    }

    changeAnimation(entity, initialAnimState, resources);
}
// 功能：根据行为意图、物理规则和碰撞结果更新实体移动状态。
void MovementHandle::update(
    Entity& self,
    BehaviorIntent intent,
    vector<Entity>& entitys,
    int selfIndex,
    TileMap& tileMap,
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
    // 朝向是真实游戏状态，后续会影响攻击、交互、射线检测等逻辑，因此由移动层更新。
    if (inputX < -EPS)
    {
        self.setFacingDirection(LEFT);
    }
    else if (inputX > EPS)
    {
        self.setFacingDirection(RIGHT);
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
        selfIndex,
        tileMap
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
        selfIndex,
        tileMap
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
    vector<Entity>& entitys,
    int selfIndex,
    TileMap& tileMap
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

    for (int i = 0; i < (int)entitys.size(); i++)
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


    for (int row = 0; row < tileMap.getRows(); row++)
    {
        for (int col = 0; col < tileMap.getCols(); col++)
        {
            TileCollisionType collisionType = tileMap.getTileCollisionType(row, col);

            if (collisionType == TILE_COLLISION_NONE)
            {
                continue;
            }

            if (collisionType == TILE_COLLISION_FULL_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY)
            {
                continue;
            }

            RectBox tileBox = tileMap.getTileCollisionWorldBox(row, col);

            if (!isRangeOverlapping(myBox.bottom, myBox.top, tileBox.bottom, tileBox.top))
            {
                continue;
            }

            if (moveX > 0)
            {
                if (tileBox.left >= myBox.right - EPS)
                {
                    double distance = tileBox.left - myBox.right;

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
                if (tileBox.right <= myBox.left + EPS)
                {
                    double distance = tileBox.right - myBox.left;

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
    }


    return allowedMove;
}
// 功能：计算实体在 Y 轴上不会穿透阻挡物的最大允许位移。
double CollisionHandle::getAllowedMoveY(
    Entity& self,
    double moveY,
    vector<Entity>& entitys,
    int selfIndex,
    TileMap& tileMap
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

    for (int i = 0; i < (int)entitys.size(); i++)
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


    for (int row = 0; row < tileMap.getRows(); row++)
    {
        for (int col = 0; col < tileMap.getCols(); col++)
        {
            TileCollisionType collisionType = tileMap.getTileCollisionType(row, col);

            if (collisionType == TILE_COLLISION_NONE)
            {
                continue;
            }

            if (collisionType == TILE_COLLISION_FULL_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY ||
                collisionType == TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY)
            {
                if (moveY >= 0)
                {
                    continue;
                }
            }

            RectBox tileBox = tileMap.getTileCollisionWorldBox(row, col);


            if (!isRangeOverlapping(myBox.left, myBox.right, tileBox.left, tileBox.right))
            {
                continue;
            }

            if (moveY > 0)
            {
                if (tileBox.bottom >= myBox.top - EPS)
                {
                    double distance = tileBox.bottom - myBox.top;

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
                if (tileBox.top <= myBox.bottom + EPS)
                {
                    double distance = tileBox.top - myBox.bottom;

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
 // 当前仍然用 vector 下标作为临时目标标识，后续真正 erase 实体时需要同步修正。
// 功能：切换相机当前跟随的实体下标。
void setCameraFollowTarget(int newTargetIndex, vector<Entity>& entitys)
{
    int entityCount = (int)entitys.size();

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
 // 数据流：Level::updateCamera -> updateCameraFollow -> updateZoom -> followSmooth
// 功能：根据跟随目标、鼠标偏移和缩放输入更新相机。
void updateCameraFollow(
    vector<Entity>& entitys,
    int worldWidth,
    int worldHeight,
    int mouseOffsetX,
    int mouseOffsetY
)
{
    int entityCount = (int)entitys.size();

    if (entityCount <= 0)
    {
        return;
    }

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

    // 先更新 zoom，再用新的可见视口范围限制 camera center。
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
    // 用鼠标屏幕偏移除以 zoom，把屏幕像素距离还原成世界距离。
    double offsetWorldX = mouseOffsetX / gCamera.zoom * lookStrength;

    // 注意这里要反过来：
    // 鼠标在屏幕上方时 mouseOffsetY 是负数，
    // 但是世界坐标里向上应该是正数。
    // 用负号翻转屏幕 Y 方向，再除以 zoom 得到世界 Y 偏移。
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


// DebugPanelData：
// Debug 面板一帧要显示的数据快照。
struct DebugPanelData
{
    int targetIndex;

    double entityX;
    double entityY;

    int entityScreenX;
    int entityScreenY;

    int renderEntityCount;
    int renderTileCount;

    double cameraCenterX;
    double cameraCenterY;
    double cameraZoom;

    double viewLeft;
    double viewRight;
    double viewBottom;
    double viewTop;

    DebugPanelData()
    {
        targetIndex = -1;

        entityX = 0;
        entityY = 0;

        entityScreenX = 0;
        entityScreenY = 0;

        renderEntityCount = 0;
        renderTileCount = 0;

        cameraCenterX = 0;
        cameraCenterY = 0;
        cameraZoom = 1.0;

        viewLeft = 0;
        viewRight = 0;
        viewBottom = 0;
        viewTop = 0;
    }
};




// Renderer：
 // 统一管理当前关卡中的可渲染对象。
 // 它负责把 sprite / tile / UI 等数据绘制到屏幕，并集中处理实体调试碰撞框绘制。
 // 实体自身不再直接调用 EasyX 绘图函数。
class Renderer
{
private:

	// 是否显示实体逻辑碰撞盒。
	bool showCollisionBox;

	// 是否显示 tile 逻辑碰撞盒。
	bool showTileCollisionBox;

    // 是否显示所有可绘制对象的屏幕绘制边界。
    bool showRenderBounds;


    // 功能：从图集中裁剪指定区域，并以 Alpha 混合绘制到屏幕目标矩形。
    void drawImageTileAlpha(
        int destX,
        int destY,
        int destW,
        int destH,
        IMAGE* imageSource,
        int srcX,
        int srcY,
        int srcW,
        int srcH
    )
    {
        if (imageSource == NULL)
        {
            return;
        }

        if (destW <= 0 || destH <= 0 || srcW <= 0 || srcH <= 0)
        {
            return;
        }

        BLENDFUNCTION blend;
        blend.BlendOp = AC_SRC_OVER;
        blend.BlendFlags = 0;
        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat = AC_SRC_ALPHA;

        AlphaBlend(
            GetImageHDC(NULL),
            destX,
            destY,
            destW,
            destH,
            GetImageHDC(imageSource),
            srcX,
            srcY,
            srcW,
            srcH,
            blend
        );
    }

    // 功能：绘制实体当前世界碰撞盒的调试矩形。
    void drawEntityCollisionBox(Entity& entity)
    {
        RectBox box = entity.getWorldCollisionBox();

        if (entity.hasCollisionState())
        {
            setlinecolor(RED);
        }
        else
        {
            setlinecolor(GREEN);
        }

        int screenLeft = gCamera.worldToScreenX(box.left);
        int screenRight = gCamera.worldToScreenX(box.right);

        int screenTop = gCamera.worldToScreenY(box.top);
        int screenBottom = gCamera.worldToScreenY(box.bottom);

        rectangle(screenLeft, screenTop, screenRight, screenBottom);
    }

    // 功能：在屏幕坐标中绘制渲染对象的实际绘制边界。
    // 这个矩形表示图像最终画到屏幕上的范围，不等同于逻辑碰撞盒。
    void drawRenderBounds(int x, int y, int w, int h, COLORREF color)
    {
        if (!showRenderBounds)
        {
            return;
        }

        if (w <= 0 || h <= 0)
        {
            return;
        }

        setlinecolor(color);
        rectangle(x, y, x + w, y + h);
    }


public:
	// 功能：初始化渲染器的调试绘制开关。
	Renderer()
	{
		showCollisionBox = true;
		showTileCollisionBox = false;
		showRenderBounds = false;
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

    // 功能：切换可绘制对象的绘制边界框显示状态。
    void toggleRenderBounds()
    {
        showRenderBounds = !showRenderBounds;
    }

    // 功能：绘制当前关卡多层视差背景。
    // parallaxOffsetX 表示相机中心相对初始中心的累计水平位移。
    // cameraZoom 表示当前真实相机 zoom，用于让不同远近的背景层按不同强度响应缩放。
    // 注意：这里不再读取旧的视口左边界；视差只使用 centerX 的真实位移，避免被 zoom 改变污染。
    void drawBackground(IMAGE backgroundLayers[], int layerCount, double parallaxOffsetX, double cameraZoom)
    {
        // 数值越大，越靠近前景，横向移动越明显。
        double parallaxFactors[5] = { 0.0, 0.04, 0.12, 0.25, 0.4 };
        // 数值越大，这一层越明显地响应 camera zoom。
        // 最远天空层通常不明显缩放；近景层更接近场景元素。
        double zoomFactors[5] = { 0.0, 0.25, 0.55, 0.85, 1.0 };

        if (layerCount > 5)
        {
            layerCount = 5;
        }

        for (int i = 0; i < layerCount; i++)
        {
            int imageW = backgroundLayers[i].getwidth();
            int imageH = backgroundLayers[i].getheight();

            if (imageW <= 0 || imageH <= 0)
            {
                continue;
            }

            // 计算这一层自己的 zoom。
            // 不是所有背景层都必须 100% 跟随 camera zoom。
            double layerZoom = 1.0 + (cameraZoom - 1.0) * zoomFactors[i];

            // 防止 zoom out 时背景缩小露出黑边。
            // 如果你想让背景完全像世界物体一样缩小，可以删掉这个 if。
            if (layerZoom < 1.0)
            {
                layerZoom = 1.0;
            }

            int drawW = (int)(imageW * layerZoom);
            int drawH = (int)(imageH * layerZoom);

            if (drawW < 1)
            {
                drawW = 1;
            }

            if (drawH < 1)
            {
                drawH = 1;
            }

            // 背景平移只使用相机中心的真实位移；layerZoom 只负责把该层自身缩放后的偏移换算成屏幕距离。
            // 不直接乘全局 cameraZoom，避免角色不动但缩放时背景发生横向滑动。
            double screenOriginX =
                -parallaxOffsetX * parallaxFactors[i] * layerZoom
                + WINDOW_WIDTH / 2.0
                - drawW / 2.0;

            int baseX = (int)fmod(screenOriginX, (double)drawW);

            if (baseX > 0)
            {
                baseX -= drawW;
            }

            // 纵向先保持屏幕中心缩放。
            // 这样 zoom in 时背景从屏幕中心向外放大。
            int drawY = (WINDOW_HEIGHT - drawH) / 2;

            for (int drawX = baseX; drawX < WINDOW_WIDTH; drawX += drawW)
            {
                if (i == 0)
                {
                    putimage(drawX, drawY, drawW, drawH, &backgroundLayers[i], 0, 0);
                }
                else
                {
                    putimage_alpha(drawX, drawY, drawW, drawH, &backgroundLayers[i]);
                }

                // 背景层可能会横向平铺；这里显示的是每一块平铺图片的实际绘制范围。
                drawRenderBounds(
                    drawX,
                    drawY,
                    drawW,
                    drawH,
                    RGB(120, 160, 255)
                );
            }
        }
    }

    // 功能：根据 TileInstance 生成通用 sprite，并交给统一 sprite 绘制接口。
    void drawTileInstance(TileMap& tileMap, const TileInstance& tile)
    {
        sprite tileSprite = tileMap.buildSpriteFromTileInstance(tile);

        drawSprite(
            tileSprite,
            RGB(255, 220, 0)
        );
    }


    // 功能：逐个绘制当前地图中的 tile 实例，并根据开关绘制 tile 调试碰撞框。
    void drawTileMap(TileMap& tileMap)
    {
        const vector<TileInstance>& tileInstances = tileMap.getTileInstances();

        for (int i = 0; i < (int)tileInstances.size(); i++)
        {
            drawTileInstance(tileMap, tileInstances[i]);
        }

        if (showTileCollisionBox)
        {
            tileMap.drawDebugCollisionBoxes();
        }
    }

    // 功能：根据 sprite 自身保存的世界绘制数据绘制单帧图像。
    void drawSprite(const sprite& targetSprite, COLORREF renderBoundsColor = RGB(0, 220, 255))
    {
        if (!targetSprite.visible)
        {
            return;
        }

        if (targetSprite.imageSource == NULL)
        {
            return;
        }

        if (targetSprite.srcW <= 0 || targetSprite.srcH <= 0)
        {
            return;
        }

        if (targetSprite.worldDrawW <= 0 || targetSprite.worldDrawH <= 0)
        {
            return;
        }

        // 用 sprite 世界中心点和世界绘制尺寸，计算世界绘制矩形。
        double worldLeft = targetSprite.worldCenterX - targetSprite.worldDrawW / 2.0;
        double worldTop = targetSprite.worldCenterY + targetSprite.worldDrawH / 2.0;

        // 把世界绘制矩形转换为屏幕左上角。
        int drawX = gCamera.worldToScreenX(worldLeft);
        int drawY = gCamera.worldToScreenY(worldTop);

        // 把世界绘制尺寸转换为屏幕绘制尺寸。
        int screenDrawW = gCamera.worldSizeToScreen(targetSprite.worldDrawW);
        int screenDrawH = gCamera.worldSizeToScreen(targetSprite.worldDrawH);

        if (screenDrawW < 1)
        {
            screenDrawW = 1;
        }

        if (screenDrawH < 1)
        {
            screenDrawH = 1;
        }

        // 根据源图裁剪矩形和屏幕目标矩形完成 Alpha 混合绘制。
        drawImageTileAlpha(
            drawX,
            drawY,
            screenDrawW,
            screenDrawH,
            targetSprite.imageSource,
            targetSprite.srcX,
            targetSprite.srcY,
            targetSprite.srcW,
            targetSprite.srcH
        );

        // sprite 绘制边界来自 sprite 自身的世界绘制数据。
        drawRenderBounds(
            drawX,
            drawY,
            screenDrawW,
            screenDrawH,
            renderBoundsColor
        );
    }


	// 功能：绘制实体列表中的所有存活实体，并根据开关绘制实体调试碰撞框。
	void drawEntities(vector<Entity>& entitys)
	{
		for (int i = 0; i < (int)entitys.size(); i++)
		{
			if (!entitys[i].getIsAlive())
			{
				continue;
			}

            drawSprite(entitys[i].getSprite());
			if (showCollisionBox)
			{
                drawEntityCollisionBox(entitys[i]);
			}
		}
	}

    // 功能：绘制一个通用 UIElement 面板。
    void drawUIElementPanel(const UIElement& element)
    {
        if (!element.isVisible())
        {
            return;
        }

        drawUIBox(
            element.getBox(),
            RGB(255, 255, 255),
            RGB(180, 180, 180)
        );
    }

    // 功能：绘制 Debug 面板中的实体数据区。
    void drawDebugEntitySectionText(const UIElement& content, DebugPanelData data)
    {
        if (!content.isVisible())
        {
            return;
        }

        UIBox box = content.getBox();

        setbkmode(TRANSPARENT);
        settextcolor(RGB(40, 40, 40));
        settextstyle(18, 0, _T("Mojangles"));

        int x = box.x;
        int y = box.y;
        int lineH = 22;

        TCHAR text[128];

        _stprintf_s(text, _T("Debug Target"));
        outtextxy(x, y, text);
        y += lineH;

        _stprintf_s(text, _T("Entity Index: %d"), data.targetIndex);
        outtextxy(x, y, text);
        y += lineH;

        _stprintf_s(text, _T("World Pos: %.1f, %.1f"), data.entityX, data.entityY);
        outtextxy(x, y, text);
        y += lineH;

        _stprintf_s(text, _T("Screen Pos: %d, %d"), data.entityScreenX, data.entityScreenY);
        outtextxy(x, y, text);
    }

    // 功能：绘制 Debug 面板中的渲染数据区。
    void drawDebugRenderSectionText(const UIElement& content, DebugPanelData data)
    {
        if (!content.isVisible())
        {
            return;
        }

        UIBox box = content.getBox();

        setbkmode(TRANSPARENT);
        settextcolor(RGB(40, 40, 40));
        settextstyle(18, 0, _T("Mojangles"));

        int x = box.x;
        int y = box.y;
        int lineH = 22;

        TCHAR text[128];

        _stprintf_s(text, _T("Render"));
        outtextxy(x, y, text);
        y += lineH;

        _stprintf_s(text, _T("Entities: %d"), data.renderEntityCount);
        outtextxy(x, y, text);
        y += lineH;

        _stprintf_s(text, _T("Tiles: %d"), data.renderTileCount);
        outtextxy(x, y, text);
    }

    // 功能：绘制 Debug 面板中的相机数据区。
    void drawDebugCameraSectionText(const UIElement& content, DebugPanelData data)
    {
        if (!content.isVisible())
        {
            return;
        }

        UIBox box = content.getBox();

        setbkmode(TRANSPARENT);
        settextcolor(RGB(40, 40, 40));
        settextstyle(18, 0, _T("Mojangles"));

        int x = box.x;
        int y = box.y;
        int lineH = 22;

        TCHAR text[128];

        _stprintf_s(text, _T("Camera"));
        outtextxy(x, y, text);
        y += lineH;

        _stprintf_s(text, _T("Center: %.1f, %.1f"), data.cameraCenterX, data.cameraCenterY);
        outtextxy(x, y, text);
        y += lineH;

        _stprintf_s(text, _T("Zoom: %.2f"), data.cameraZoom);
        outtextxy(x, y, text);
        y += lineH;

        _stprintf_s(text, _T("View L/R: %.1f / %.1f"), data.viewLeft, data.viewRight);
        outtextxy(x, y, text);
        y += lineH;

        _stprintf_s(text, _T("View B/T: %.1f / %.1f"), data.viewBottom, data.viewTop);
        outtextxy(x, y, text);
    }
};

// Level：
 // 当前关卡/场景管理器。
 // 它持有当前关卡的地图、背景、实体列表、UI 面板和各种 Handle。
 // 它不应该亲自写复杂的移动/碰撞细节，而是负责“调度顺序”：
 //   init()   加载关卡内容
 //   update() 每帧按顺序调度输入、实体、相机、事件、UI
 //   draw()   按层级绘制背景、地图、实体、UI
class Level
{
private:

    ResourceManager resources;

    TileMap tileMap;
    IMAGE backgrounds[5];

    // 当前关卡实体列表。使用 vector 取代固定数组，为后续动态生成和清理实体做准备。
    vector<Entity> entitys;

    // 当前关卡 UI 元素管理器，负责维护 UIElement 的父子关系和位置更新。
    UIManager uiManager;

    // Debug 面板父元素在 UIManager 中的下标。
    int debugPanelIndex;

    // Debug 面板下的三个逻辑子区域；它们共享父级面板，但可以独立控制显示状态。
    int debugEntitySectionIndex;
    int debugRenderSectionIndex;
    int debugCameraSectionIndex;

    Renderer renderer;

    int controlTargerIndex;

    PlayerController playerController;
    MovementHandle movementHandle;
    CollisionHandle collisionHandle;

    int worldWidth;
    int worldHeight;

    // 背景视差专用相机位置。
    // 它记录的是真实摄像机中心 gCamera.centerX，
    // 也就是背景层用于计算累计水平位移的稳定锚点。
    // 当摄像机被世界边界限制住时，这个值不会继续变化，背景也不会继续移动。
    double parallaxCameraX;

    // 背景视差初始参考点。
    // 用来计算摄像机从初始位置开始实际移动了多少。
    double parallaxOriginX;

    // 功能：根据当前相机跟随目标生成 Debug 面板数据。
    DebugPanelData buildDebugPanelData()
    {
        DebugPanelData data;

        data.targetIndex = gCameraFollowTargetIndex;

        if (
            gCameraFollowTargetIndex >= 0 &&
            gCameraFollowTargetIndex < (int)entitys.size()
            )
        {
            Entity& target = entitys[gCameraFollowTargetIndex];

            data.entityX = target.getX();
            data.entityY = target.getY();

            data.entityScreenX = gCamera.worldToScreenX(target.getX());
            data.entityScreenY = gCamera.worldToScreenY(target.getY());
        }

        data.renderEntityCount = countRenderableEntities();
        data.renderTileCount = tileMap.getTileInstanceCount();

        data.cameraCenterX = gCamera.centerX;
        data.cameraCenterY = gCamera.centerY;
        data.cameraZoom = gCamera.zoom;

        data.viewLeft = gCamera.getViewLeft();
        data.viewRight = gCamera.getViewRight();
        data.viewBottom = gCamera.getViewBottom();
        data.viewTop = gCamera.getViewTop();

        return data;
    }

    // 功能：统计当前场景中存活且可参与绘制的实体数量。
    // 当前第一版只按 isAlive 统计，后续可进一步过滤 sprite visible / 视口裁切结果。
    int countRenderableEntities()
    {
        int count = 0;

        for (int i = 0; i < (int)entitys.size(); i++)
        {
            if (!entitys[i].getIsAlive())
            {
                continue;
            }

            count++;
        }

        return count;
    }

    // 以下历史状态缓存必须与 entitys.size() 同步，用于判断状态变化和重叠事件首次触发。
    vector<vector<bool>> lastOverlap;
    vector<bool> lastCollisionState;
    vector<bool> lastGroundState;
    vector<bool> lastSprintState;
    vector<bool> lastInAirState;
    vector<bool> lastJumpingState;
    vector<bool> lastAliveState;

public:
    // 功能：初始化关卡实体列表和默认世界尺寸。
    Level()
    {

        controlTargerIndex = 0;

        // 初始化 UI 元素下标；-1 表示当前还没有创建对应元素。
        debugPanelIndex = -1;

        debugEntitySectionIndex = -1;
        debugRenderSectionIndex = -1;
        debugCameraSectionIndex = -1;

        // 预留当前测试关卡的实体数量，避免初始化期间 vector 扩容搬移 Entity。
        entitys.reserve(20);

        entitys.emplace_back(200, 700, true, true, true, false, PLAYER, ANIM_SET_PLAYER1, 1);

        entitys.emplace_back(_T("assets\\tex\\entities\\characters\\player2.png"), 600, 1300, false, true, false, false, ENTITY, 1);

        entitys.emplace_back(_T("assets\\tex\\entities\\characters\\player3.png"), 950, 1300, false, true, true, false, ENTITY, 1);

        entitys.emplace_back(_T("assets\\tex\\entities\\characters\\player4.png"), 1300, 1300, false, true, false, false, ENTITY, 1);

        entitys.emplace_back(_T("assets\\tex\\entities\\items\\MonedaD.png"), 256, 256, false, true, false, true, COIN, 5, 1);

        entitys.emplace_back(_T("assets\\tex\\entities\\items\\MonedaP.png"), 256 + 48 + 16, 256, false, true, false, true, COIN, 5, 1);

        entitys.emplace_back(_T("assets\\tex\\entities\\items\\MonedaR.png"), 256 + (48 * 2) + (16 * 2), 256, false, true, false, true, COIN, 5, 1);

        entitys.emplace_back(_T("assets\\tex\\entities\\items\\MonedaD.png"), 5662, 1312, false, true, false, true, COIN, 5, 1);

        entitys.emplace_back(_T("assets\\tex\\entities\\items\\MonedaD.png"), 5662, 1312 + 64 + 16, false, true, false, true, COIN, 5, 1);

        entitys.emplace_back(_T("assets\\tex\\entities\\items\\MonedaD.png"), 5662, 1312 + 64 + 16 + 64 + 16, false, true, false, true, COIN, 5, 1);






        worldWidth = WINDOW_WIDTH;
        worldHeight = WINDOW_HEIGHT;

        parallaxCameraX = 0.0;
        parallaxOriginX = 0.0;
    }

    // 功能：在资源加载完成后，为所有实体同步初始动画帧。
    void initEntityAnimations()
    {
        for (int i = 0; i < (int)entitys.size(); i++)
        {
            entitys[i].initAnimationFromAnimator(resources);
        }
    }

    // 功能：初始化关卡地图、背景、UI、实体设置和历史状态缓存。
    void init()
    {
        initResources();
        initMap();
        initBackground();
        initUI();
        initEntityAnimations();
        initEntitySettings();
        initLastStates();

        // 初始化相机位置，避免第一帧背景原点和真实相机位置不同。
        if (!entitys.empty())
        {
            gCamera.followInstant(entitys[0].getX(), entitys[0].getY(), worldWidth, worldHeight);
        }

        parallaxCameraX = gCamera.centerX;
        parallaxOriginX = gCamera.centerX;
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
            5. handleUIInput() 处理 Debug UI 临时显隐输入
            6. updateCamera() 更新摄像机跟随
            7. updateDebugStates() 输出状态变化
            8. updateOverlapEvents() 处理重叠事件，如金币拾取
            9. uiManager.update() 更新 UIElement 父子定位和过渡
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

        handleControlInput(input);

        updateEntities(input);

        handleCameraInput(input);
        handleUIInput(input);
        handleRendererInput(input);

        updateCamera(input);
        updateParallaxCamera();

        updateDebugStates();

        updateOverlapEvents();
        uiManager.update();
    }

	// 功能：委托 Renderer 绘制当前关卡画面。
    void draw()
    {
        double parallaxOffsetX = parallaxCameraX - parallaxOriginX;

        renderer.drawBackground(backgrounds, 5, parallaxOffsetX, gCamera.zoom);
        renderer.drawTileMap(tileMap);
        renderer.drawEntities(entitys);

        // Debug 面板父级负责整体背景；子 section 的最终可见性由 UIManager 按父级链路判断。
        if (uiManager.isElementEffectivelyVisible(debugPanelIndex))
        {
            renderer.drawUIElementPanel(uiManager.getElement(debugPanelIndex));
        }

        DebugPanelData data = buildDebugPanelData();

        if (uiManager.isElementEffectivelyVisible(debugEntitySectionIndex))
        {
            renderer.drawDebugEntitySectionText(
                uiManager.getElement(debugEntitySectionIndex),
                data
            );
        }

        if (uiManager.isElementEffectivelyVisible(debugRenderSectionIndex))
        {
            renderer.drawDebugRenderSectionText(
                uiManager.getElement(debugRenderSectionIndex),
                data
            );
        }

        if (uiManager.isElementEffectivelyVisible(debugCameraSectionIndex))
        {
            renderer.drawDebugCameraSectionText(
                uiManager.getElement(debugCameraSectionIndex),
                data
            );
        }

    }


private:

	// 功能：加载当前关卡需要的资源。
	void initResources()
	{
		resources.loadLevelResources();
	}


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
        loadimage(&backgrounds[0], _T("assets\\tex\\maps\\Clouds 5\\1.png"), WINDOW_WIDTH, WINDOW_HEIGHT, true);
        loadimage(&backgrounds[1], _T("assets\\tex\\maps\\Clouds 5\\2.png"), WINDOW_WIDTH, WINDOW_HEIGHT, true);
        loadimage(&backgrounds[2], _T("assets\\tex\\maps\\Clouds 5\\3.png"), WINDOW_WIDTH, WINDOW_HEIGHT, true);
        loadimage(&backgrounds[3], _T("assets\\tex\\maps\\Clouds 5\\4.png"), WINDOW_WIDTH, WINDOW_HEIGHT, true);
        loadimage(&backgrounds[4], _T("assets\\tex\\maps\\Clouds 5\\5.png"), WINDOW_WIDTH, WINDOW_HEIGHT, true);
    }
    // 功能：初始化当前关卡使用的 Debug UI 面板。
    void initUI()
    {
        // Debug 面板是顶层 UI，相对于窗口右上角定位。
        UIElement debugPanel;
        debugPanel.init(420, 520, UI_TOP_RIGHT, 24, 24);

        debugPanelIndex = uiManager.addElement(debugPanel);

        // Debug Entity 区域：显示当前相机跟随目标实体的数据。
        UIElement debugEntitySection;
        debugEntitySection.init(388, 110, UI_TOP_LEFT, 16, 16);
        debugEntitySection.setParentIndex(debugPanelIndex);
        debugEntitySection.refreshTargetByParentBox(uiManager.getElement(debugPanelIndex).getBox());
        debugEntitySection.snapToTarget();
        debugEntitySectionIndex = uiManager.addElement(debugEntitySection);

        // Debug Render 区域：显示当前渲染相关数据。
        UIElement debugRenderSection;
        debugRenderSection.init(388, 90, UI_TOP_LEFT, 16, 140);
        debugRenderSection.setParentIndex(debugPanelIndex);
        debugRenderSection.refreshTargetByParentBox(uiManager.getElement(debugPanelIndex).getBox());
        debugRenderSection.snapToTarget();
        debugRenderSectionIndex = uiManager.addElement(debugRenderSection);

        // Debug Camera 区域：显示当前相机和视口数据。
        UIElement debugCameraSection;
        debugCameraSection.init(388, 180, UI_TOP_LEFT, 16, 250);
        debugCameraSection.setParentIndex(debugPanelIndex);
        debugCameraSection.refreshTargetByParentBox(uiManager.getElement(debugPanelIndex).getBox());
        debugCameraSection.snapToTarget();
        debugCameraSectionIndex = uiManager.addElement(debugCameraSection);
    }

    // 功能：设置实体 sprite 缩放、动画速度和碰撞盒缩放。
    void initEntitySettings()
    {
        entitys[0].setSpriteTransform(2.0, 2.0, 0, 0);

        entitys[3].setSpriteTransform(2.0, 2.0, 0, 0);

        entitys[4].setSpriteTransform(4.0, 4.0, 0, 0);
        entitys[5].setSpriteTransform(4.0, 4.0, 0, 0);
        entitys[6].setSpriteTransform(4.0, 4.0, 0, 0);
        entitys[7].setSpriteTransform(4.0, 4.0, 0, 0);
        entitys[8].setSpriteTransform(4.0, 4.0, 0, 0);
        entitys[9].setSpriteTransform(4.0, 4.0, 0, 0);



        entitys[0].setAnimationSpeed(3);
        entitys[4].setAnimationSpeed(3);

        entitys[0].setCollisionScale(1.2, 2);
    }

    // 功能：初始化用于检测状态变化的历史缓存。
    void initLastStates()
    {
        int entityCount = (int)entitys.size();

        // 根据当前实体数量重建缓存；以后真正 erase 实体后也需要重新同步这些容器。
        lastCollisionState.assign(entityCount, false);
        lastGroundState.assign(entityCount, false);
        lastSprintState.assign(entityCount, false);
        lastInAirState.assign(entityCount, false);
        lastJumpingState.assign(entityCount, false);
        lastAliveState.assign(entityCount, false);
        lastOverlap.assign(entityCount, vector<bool>(entityCount, false));

        for (int i = 0; i < entityCount; i++)
        {
            lastAliveState[i] = entitys[i].getIsAlive();
        }
    }

    // 功能：清理所有存活实体的本帧临时状态。
    void clearEntityFrameState()
    {
        for (int i = 0; i < (int)entitys.size(); i++)
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

        if (
            controlTargerIndex < 0 ||
            controlTargerIndex >= (int)entitys.size() ||
            !entitys[controlTargerIndex].getIsAlive()
            )
        {
            controlTargerIndex = 0;
        }


        for (int i = 0; i < (int)entitys.size(); i++)
        {
            if (!entitys[i].getIsAlive())
            {
                continue;
            }

            BehaviorIntent intent;

            if (
                controlTargerIndex >= 0 &&
                controlTargerIndex < (int)entitys.size() &&
                i == controlTargerIndex
                )
            {
                intent = playerController.makeIntent(input, entitys[i].isGod());
            }
            movementHandle.update(
                entitys[i],
                intent,
                entitys,
                i,
                tileMap,
                worldWidth,
                worldHeight,
                collisionHandle
            );

			entitys[i].updateAnimator(intent, resources);
			entitys[i].updateAnimatedSprite();
        }
    }

    // 功能：切换当前由玩家输入控制的实体下标。
    void setControlTarget(int newTargetIndex)
    {
        int entityCount = (int)entitys.size();

        if (newTargetIndex < 0 || newTargetIndex >= entityCount)
        {
            return;
        }

        if (!entitys[newTargetIndex].getIsAlive())
        {
            return;
        }

        controlTargerIndex = newTargetIndex;

        cout << "Control target changed to Entity "
            << controlTargerIndex
            << endl;
    }
    // 功能：处理玩家输入控制目标切换。
    void handleControlInput(InputManager& input)
    {
        if (input.isKeyPressed('1'))
        {
            setControlTarget(0);
        }

        if (input.isKeyPressed('2'))
        {
            setControlTarget(1);
        }

        if (input.isKeyPressed('3'))
        {
            setControlTarget(2);
        }

        if (input.isKeyPressed('4'))
        {
            setControlTarget(3);
        }
    }
    // 功能：处理相机跟随目标切换输入。
    void handleCameraInput(InputManager& input)
    {
        if (input.isKeyPressed(VK_F1))
        {
            setCameraFollowTarget(0, entitys);
        }

        if (input.isKeyPressed(VK_F2))
        {
            setCameraFollowTarget(1, entitys);
        }

        if (input.isKeyPressed(VK_F3))
        {
            setCameraFollowTarget(2, entitys);
        }

        if (input.isKeyPressed(VK_F4))
        {
            setCameraFollowTarget(3, entitys);
        }
    }

    // 功能：处理 Debug UI 临时显隐控制输入。
    void handleUIInput(InputManager& input)
    {
        // 以下 F8-F11 仅用于当前阶段验证 Debug UI 层级、显隐状态和 section 动画。
        // 后续正式 UI 交互应改由按钮、配置面板或 Debug 菜单自身控制。
        if (input.isKeyPressed(VK_F8))
        {
            toggleDebugPanelVisible();
            cout << "Toggle debug panel." << endl;
        }

        if (input.isKeyPressed(VK_F9))
        {
            toggleDebugEntitySectionVisible();
            cout << "Toggle debug entity section." << endl;
        }

        if (input.isKeyPressed(VK_F10))
        {
            toggleDebugRenderSectionVisible();
            cout << "Toggle debug render section." << endl;
        }

        if (input.isKeyPressed(VK_F11))
        {
            toggleDebugCameraSectionVisible();
            cout << "Toggle debug camera section." << endl;
        }

    }

    // 功能：切换整个 Debug 面板的显示状态。
    void toggleDebugPanelVisible()
    {
        toggleUIElementVisible(debugPanelIndex);
    }

    // 功能：切换 Debug 实体信息区。显示时从左侧滑入，隐藏时向右侧滑出。
    void toggleDebugEntitySectionVisible()
    {
        UIElement& element = uiManager.getElement(debugEntitySectionIndex);

        if (element.isVisible())
        {
            uiManager.hideElementAnimatedToOffset(debugEntitySectionIndex, 40, 0);
        }
        else
        {
            uiManager.showElementAnimatedFromOffset(debugEntitySectionIndex, -40, 0);
        }
    }

    // 功能：切换 Debug 渲染信息区。显示时从右侧滑入，隐藏时向左侧滑出。
    void toggleDebugRenderSectionVisible()
    {
        UIElement& element = uiManager.getElement(debugRenderSectionIndex);

        if (element.isVisible())
        {
            uiManager.hideElementAnimatedToOffset(debugRenderSectionIndex, -40, 0);
        }
        else
        {
            uiManager.showElementAnimatedFromOffset(debugRenderSectionIndex, 40, 0);
        }
    }

    // 功能：切换 Debug 相机信息区。显示时从下方滑入，隐藏时向上方滑出。
    void toggleDebugCameraSectionVisible()
    {
        UIElement& element = uiManager.getElement(debugCameraSectionIndex);

        if (element.isVisible())
        {
            uiManager.hideElementAnimatedToOffset(debugCameraSectionIndex, 0, -40);
        }
        else
        {
            uiManager.showElementAnimatedFromOffset(debugCameraSectionIndex, 0, 40);
        }
    }

    // 功能：切换指定 UI 元素自身的显示状态。
    // 这个函数只改变当前元素，不递归修改子元素；父级覆盖由 isElementEffectivelyVisible() 处理。
    void toggleUIElementVisible(int elementIndex)
    {
        if (!uiManager.isValidIndex(elementIndex))
        {
            return;
        }

        UIElement& element = uiManager.getElement(elementIndex);

        if (element.isVisible())
        {
            element.hide();
        }
        else
        {
            uiManager.showElementInstant(elementIndex);
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
			cout << "Toggle map tile edge." << endl;

		}
        if (input.isKeyPressed(VK_F7))
        {
            renderer.toggleRenderBounds();
            cout << "Toggle render bounds." << endl;
        }
	}

    // 功能：根据输入状态更新相机跟随。
    void updateCamera(InputManager& input)
    {
        updateCameraFollow(
            entitys,
            worldWidth,
            worldHeight,
            //input.getMouseOffsetX(),
            //input.getMouseOffsetY()
            0,0
        );
    }


    // 功能：更新背景视差专用相机位置。
    // 注意：这里记录的是摄像机最终中心位置，而不是角色位置。
    // 因此当摄像机被世界边界限制住时，背景也会停止滚动。
    void updateParallaxCamera()
    {
        parallaxCameraX = gCamera.centerX;
    }

    // 功能：检测实体状态变化并输出调试信息。
    void updateDebugStates()
    {
        for (int i = 0; i < (int)entitys.size(); i++)
        {
            if (entitys[i].hasCollisionState() && !lastCollisionState[i])
            {
                cout << "Entity " << i << " collision state started." << endl;
            }

            lastCollisionState[i] = entitys[i].hasCollisionState();
        }

        for (int i = 0; i < (int)entitys.size(); i++)
        {
            bool nowAlive = entitys[i].getIsAlive();

            if (!nowAlive && lastAliveState[i])
            {
                cout << "Entity " << i << " died." << endl;
            }

            lastAliveState[i] = nowAlive;
        }

        for (int i = 0; i < (int)entitys.size(); i++)
        {
            if (entitys[i].isOnGround() && !lastGroundState[i])
            {
                cout << "Entity " << i << " is on ground." << endl;
            }

            lastGroundState[i] = entitys[i].isOnGround();
        }

        for (int i = 0; i < (int)entitys.size(); i++)
        {
            if (entitys[i].isInAir() && !lastInAirState[i])
            {
                cout << "Entity " << i << " is in air." << endl;
            }

            lastInAirState[i] = entitys[i].isInAir();
        }

        for (int i = 0; i < (int)entitys.size(); i++)
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

        for (int i = 0; i < (int)entitys.size(); i++)
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
        for (int i = 0; i < (int)entitys.size(); i++)
        {
            for (int j = i + 1; j < (int)entitys.size(); j++)
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

    // 加载全局字体，供 UI 和调试信息使用。
    loadUIFont();



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
