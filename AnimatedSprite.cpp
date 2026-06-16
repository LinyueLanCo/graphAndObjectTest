#include "AnimatedSprite.h"

// 功能：初始化序列帧动画的默认播放参数。
animatedSprite::animatedSprite()
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

    imageSource = NULL;
}

// 功能：判断非循环动画是否已经播放结束。
bool animatedSprite::isFinished()
{
    return !isLoop && !isPlaying;
}

// 功能：按显式帧尺寸加载序列帧图片。
void animatedSprite::load(const TCHAR* path, int frameWidth, int frameHeight, int frameCount)
{
    image.load(path);
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
    isPlaying = true;
}

// 功能：按帧数自动平均切分横向序列帧图片。
void animatedSprite::load(const TCHAR* path, int newFrameCount)
{
    image.load(path);
    imageSource = &image;

    frameCount = newFrameCount;

    if (frameCount < 1)
    {
        frameCount = 1;
    }

    frameWidth = image.getWidth() / frameCount;
    frameHeight = image.getHeight();

    sourceStartX = 0;
    sourceStartY = 0;
    frameSpacingX = 0;
    frameSpacingY = 0;
    frameColumns = frameCount;

    currentFrame = 0;
    frameTimer = 0;
    isPlaying = true;
}

// 功能：绑定已经由 AnimationClipManager 提供的动画片段。
void animatedSprite::setClip(AnimationClip clip)
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

    if (clip.frameWidth > 0)
    {
        frameWidth = clip.frameWidth;
    }
    else
    {
        // 用图片总宽度除以帧数，得到旧横向单行动画的单帧宽度。
        frameWidth = imageSource->getWidth() / frameCount;
    }

    if (clip.frameHeight > 0)
    {
        frameHeight = clip.frameHeight;
    }
    else
    {
        // 用图片总高度作为单帧高度，兼容旧横向单行动画。
        frameHeight = imageSource->getHeight();
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
int animatedSprite::getFrameWidth()
{
    return frameWidth;
}

// 功能：获取当前动画单帧高度。
int animatedSprite::getFrameHeight()
{
    return frameHeight;
}

// 功能：设置动画帧切换间隔。
void animatedSprite::setSpeed(int frameInterval)
{
    if (frameInterval < 1)
    {
        frameInterval = 1;
    }

    this->frameInterval = frameInterval;
}

// 功能：设置动画是否循环播放。
void animatedSprite::setLoop(bool value)
{
    isLoop = value;
}

// 功能：停止当前动画播放。
void animatedSprite::stop()
{
    isPlaying = false;
}

// 功能：重置动画到第一帧并清空计时器。
void animatedSprite::reset()
{
    currentFrame = 0;
    frameTimer = 0;
}

// 功能：推进动画帧计时并在需要时切换当前帧。
void animatedSprite::update()
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
        frameTimer = 0;
        currentFrame++;

        if (currentFrame >= frameCount)
        {
            if (isLoop)
            {
                currentFrame = 0;
            }
            else
            {
                currentFrame = frameCount - 1;
                isPlaying = false;
            }
        }
    }
}

// 功能：将当前动画帧的源图裁剪数据写入基础 sprite，供 Renderer 绘制。
void animatedSprite::writeCurrentFrameTo(sprite& targetSprite)
{
    if (imageSource == NULL)
    {
        targetSprite.setSource(NULL, 0, 0, 0, 0);
        return;
    }

    if (frameCount <= 0 || frameWidth <= 0 || frameHeight <= 0)
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

    // 用当前帧序号换算行列，再按起点、单帧尺寸和间距计算源图裁剪坐标。
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
