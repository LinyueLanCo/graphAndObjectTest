#pragma once
#include "Config.h"

// 引入标准库的 map (红黑树字典) 和 memory (智能指针库)。
// 1. map：为什么用它？因为资源表是用枚举（ImageResourceId / TextResourceId）做 Key 的。
//    使用 std::map 能实现“从 ID 到图片路径”或“从 ID 到已加载对象”的结构化映射，
//    它自动按照 Key 排序，提供了非常稳定（时间复杂度 O(log N)）的检索性能，使用极其方便。
// 2. memory (特别是 std::unique_ptr)：为什么用它？因为加载的 Image2D 图像资源需要动态分配。
//    使用 std::unique_ptr 独占型智能指针来接管动态分配出来的 Image2D 实例，
//    借助 C++ 的 RAII 机制，在 map 被 clear 或是 ResourceManager 析构时自动完成 delete 析构，
//    不再需要手写 delete 循环，从根本上杜绝了内存泄漏隐患。
#include <unordered_map>
#include <memory>
#include <string>

#include "Image2D.h"

// ResourceManager：
// 当前关卡图片及数据资源管理器。
// 它根据字符串唯一键（Name）注册和加载图片与文本资源。
class ResourceManager
{
private:
    // 注册表：存储“图片资源键名”到“图片相对路径”的映射。
    std::unordered_map<std::string, std::basic_string<TCHAR>> imagePaths;

    // 缓存表：存储“图片资源键名”到“智能指针所托管的 Image2D 图像对象”的映射。
    // 在 map 被 clear 或是 ResourceManager 销毁时，所有被智能指针托管的对象都会被自动释放。
    std::unordered_map<std::string, std::unique_ptr<Image2D>> images;

    // 注册表：存储“文本资源键名”到“文本相对路径”的映射。
    std::unordered_map<std::string, std::basic_string<TCHAR>> textPaths;

    // 缓存表：存储“文本资源键名”到“已读取的文本具体内容（std::string）”的映射。
    std::unordered_map<std::string, std::string> textContents;

public:
    // 功能：根据图片资源键名获取 EasyX 图片指针。
    IMAGE* getRawImage(const std::string& name);

    // 功能：注册当前关卡需要的通用图片资源路径。
    void initImageResourceTable();

    // 功能：根据图片资源键名加载一张 Image2D，并保存到资源表。
    void loadImage2D(const std::string& name);

    // 功能：加载当前资源表中注册的所有通用图片资源。
    void loadImageResources();

    // 功能：根据图片资源键名获取已加载的 Image2D。
    Image2D* getImage2D(const std::string& name);

    // 功能：注册当前关卡需要的通用文本资源路径。
    void initTextResourceTable();

    // 功能：根据文本资源键名加载文本文件，并保存到缓存表。
    void loadTextFile(const std::string& name);

    // 功能：加载当前资源表中注册的所有文本资源。
    void loadTextResources();

    // 功能：根据文本资源键名获取已加载的文本内容。
    const std::string& getTextContent(const std::string& name) const;

    // 功能：加载当前关卡需要的所有资源（图片和文本）。
    void loadLevelResources();
};