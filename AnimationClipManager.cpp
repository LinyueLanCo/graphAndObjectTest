#include "AnimationClipManager.h"
#include <fstream>
#include <iostream>
#include "json.hpp"

// 功能：根据动画资源配置文件动态构建动画片段表。
bool AnimationClipManager::init(const std::string& filepath, ResourceManager& resources)
{
    clips.clear();

    std::ifstream f(filepath);
    if (!f.is_open())
    {
        std::cout << "无法打开动画配置文件: " << filepath << std::endl;
        return false;
    }

    nlohmann::json data;
    try {
        f >> data;
        f.close();
    }
    catch (const std::exception& e) {
        std::cout << "解析动画配置文件 JSON 出错: " << e.what() << std::endl;
        f.close();
        return false;
    }

    if (!data.contains("clips"))
    {
        std::cout << "动画配置文件中未找到 \"clips\" 节点" << std::endl;
        return false;
    }

    auto jsonClips = data["clips"];
    for (auto& element : jsonClips.items())
    {
        std::string clipName = element.key();
        auto& config = element.value();

        std::string imageName = config.value("image", "");
        int frameCount = config.value("frameCount", 1);
        int frameDelay = config.value("frameDelay", 4);
        bool loop = config.value("loop", true);

        Image2D* img = resources.getImage2D(imageName);
        if (img == nullptr)
        {
            std::cout << "警告：动画片段 " << clipName << " 使用了未加载的图片资源 " << imageName << std::endl;
            continue;
        }

        clips[clipName] = AnimationClip(img, frameCount, frameDelay, loop);
    }

    std::cout << "动画片段加载完毕，共加载了 " << clips.size() << " 个动画片段。" << std::endl;
    return true;
}

// 功能：根据动画资源名称获取动画片段描述。
AnimationClip AnimationClipManager::getClip(const std::string& name)
{
    auto it = clips.find(name);
    if (it == clips.end())
    {
        return AnimationClip();
    }
    return it->second;
}

