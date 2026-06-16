#include "AnimationClip.h"

// 功能：初始化一个空动画片段描述。
AnimationClip::AnimationClip()
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
AnimationClip::AnimationClip(Image2D* newImage, int newFrameCount, int newSpeed, bool newLoop)
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
AnimationClip::AnimationClip(
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
