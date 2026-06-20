#pragma once

#include <unordered_map>
#include <string>

#include "AnimationClip.h"
#include "Resource.h"

// AnimationClipManager:
// 统一管理动画片段名称（string）到 AnimationClip 的映射。
// 它根据 ResourceManager 中已经加载好的 Image2D 和 animations.json 动态创建动画片段描述。
class AnimationClipManager
{
private:
    // 动画资源表：将不同的动画片段名称关联到对应的帧切片描述（AnimationClip）。
    // 使用 std::unordered_map 实现 O(1) 的平均时间检索复杂度。
    std::unordered_map<std::string, AnimationClip> clips;

public:
    bool init(const std::string& filepath, ResourceManager& resources);
    AnimationClip getClip(const std::string& name);
};

