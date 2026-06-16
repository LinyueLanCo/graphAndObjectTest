#pragma once

#include "Resource.h"

// AnimationClip:
// 动画资源描述数据。它只定义图片引用、帧数、播放参数和 sprite sheet 裁剪规则。
// 当前播放进度由 animatedSprite / 未来的 AnimationPlayer 负责。
struct AnimationClip
{
    Image2D* image;
    int frameCount;
    int speed;
    bool loop;

    // 单帧宽高。为 0 时由动画播放器按旧的横向单行规则自动计算。
    int frameWidth;
    int frameHeight;

    // 当前 clip 在 sprite sheet 中的起始裁剪坐标。
    int sourceStartX;
    int sourceStartY;

    // 相邻帧之间的横向和纵向间隔。
    int frameSpacingX;
    int frameSpacingY;

    // 每行帧数。为 0 时暂时按横向单行动画处理。
    int frameColumns;

    AnimationClip();
    AnimationClip(Image2D* newImage, int newFrameCount, int newSpeed, bool newLoop);
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
    );
};
