#pragma once

#include "AnimationClip.h"
#include "Sprite.h"

// AnimationPlayer:
// 序列帧动画播放器。
// 它根据 AnimationClip 绑定图片资源，推进 currentFrame，并把当前帧写入 sprite。
class AnimationPlayer
{
private:
    Image2D image;
    Image2D* imageSource;

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

    int frameInterval;
    int frameTimer;

public:
    AnimationPlayer();

    bool isFinished();
    void load(const TCHAR* path, int frameWidth, int frameHeight, int frameCount);
    void load(const TCHAR* path, int newFrameCount);
    void setClip(AnimationClip clip);

    int getFrameWidth();
    int getFrameHeight();

    void setSpeed(int frameInterval);
    void setLoop(bool value);
    void stop();
    void reset();
    void update();
    void writeCurrentFrameTo(sprite& targetSprite);
};
