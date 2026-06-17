#pragma once

// 引入 std::map（红黑树关联映射表）。
// 我们在这里需要通过动画 ID（AnimationId 属性枚举）能瞬间找到对应的动画片段（AnimationClip 实例描述信息）。
// 使用 std::map 存储可以确保我们以 O(log N) 的对数级别时间复杂度极速检索各个动画资源。
#include <map>

#include "AnimationClip.h"
#include "AnimationTypes.h"
#include "Resource.h"

// AnimationClipManager:
// 统一管理 AnimationId 到 AnimationClip 的映射。
// 它不加载图片，只根据 ResourceManager 中已经加载好的 Image2D 创建动画片段描述。
class AnimationClipManager
{
private:
    // 动画资源表：将不同的动画状态/类型枚举（AnimationId）关联到对应的帧切片描述（AnimationClip）。
    // 在这里使用 std::map 可以极其方便、安全地管理游戏中的所有独立动画片段资源。
    map<AnimationId, AnimationClip> clips;

public:
    void init(ResourceManager& resources);
    AnimationClip getClip(AnimationId id);
};
