#pragma once

#include "Sprite.h"
#include "Resource.h"

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
 // 会通过 getPlayerAnimationId 转换成 AnimationId，再向 AnimationClipManager 请求 AnimationClip。
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
    Image2D* image;
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

    // 功能：按图片资源、帧数、速度和循环标记创建动画片段描述。
    AnimationClip(Image2D* newImage, int newFrameCount, int newSpeed, bool newLoop)
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
        Image2D* newImage,
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



AnimationId getPlayerAnimationId(AnimationState state);
AnimationId getAnimationId(AnimationSetId setId, AnimationState state);

// AnimationClipManager：
// 统一管理 AnimationId 到 AnimationClip 的映射。
// 它不加载图片，只根据 ResourceManager 中已经加载好的 Image2D 创建动画片段描述。
class AnimationClipManager
{
private:
    map<AnimationId, AnimationClip> clips;

public:
    // 功能：根据资源管理器中已加载的图片创建当前关卡的动画片段表。
    void init(ResourceManager& resources)
    {
        clips.clear();

        clips[ANIM_ID_PLAYER_IDLE_L] =
            AnimationClip(resources.getImage2D(IMG_PLAYER_IDLE_L), 8, 3, true);

        clips[ANIM_ID_PLAYER_IDLE_R] =
            AnimationClip(resources.getImage2D(IMG_PLAYER_IDLE_R), 8, 3, true);

        clips[ANIM_ID_PLAYER_WALK_L] =
            AnimationClip(resources.getImage2D(IMG_PLAYER_WALK_L), 8, 3, true);

        clips[ANIM_ID_PLAYER_WALK_R] =
            AnimationClip(resources.getImage2D(IMG_PLAYER_WALK_R), 8, 3, true);

        clips[ANIM_ID_PLAYER_RUN_L] =
            AnimationClip(resources.getImage2D(IMG_PLAYER_RUN_L), 8, 3, true);

        clips[ANIM_ID_PLAYER_RUN_R] =
            AnimationClip(resources.getImage2D(IMG_PLAYER_RUN_R), 8, 3, true);

        clips[ANIM_ID_PLAYER_JUMP_START_L] =
            AnimationClip(resources.getImage2D(IMG_PLAYER_JUMP_START_L), 8, 2, false);

        clips[ANIM_ID_PLAYER_JUMP_START_R] =
            AnimationClip(resources.getImage2D(IMG_PLAYER_JUMP_START_R), 8, 2, false);

        clips[ANIM_ID_PLAYER_JUMP_LOOP_L] =
            AnimationClip(resources.getImage2D(IMG_PLAYER_JUMP_LOOP_L), 8, 3, true);

        clips[ANIM_ID_PLAYER_JUMP_LOOP_R] =
            AnimationClip(resources.getImage2D(IMG_PLAYER_JUMP_LOOP_R), 8, 3, true);

        clips[ANIM_ID_PLAYER_JUMP_END_L] =
            AnimationClip(resources.getImage2D(IMG_PLAYER_JUMP_END_L), 8, 2, false);

        clips[ANIM_ID_PLAYER_JUMP_END_R] =
            AnimationClip(resources.getImage2D(IMG_PLAYER_JUMP_END_R), 8, 2, false);
    }

    // 功能：根据动画资源 ID 获取动画片段描述。
    AnimationClip getClip(AnimationId id)
    {
        if (clips.find(id) == clips.end())
        {
            return AnimationClip();
        }

        return clips[id];
    }
};
