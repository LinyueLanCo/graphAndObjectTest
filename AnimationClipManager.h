#pragma once

#include "AnimationClip.h"
#include "AnimationTypes.h"
#include "Resource.h"

// AnimationClipManager:
// 统一管理 AnimationId 到 AnimationClip 的映射。
// 它不加载图片，只根据 ResourceManager 中已经加载好的 Image2D 创建动画片段描述。
class AnimationClipManager
{
private:
    map<AnimationId, AnimationClip> clips;

public:
    void init(ResourceManager& resources);
    AnimationClip getClip(AnimationId id);
};
