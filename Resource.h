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
#include <map>
#include <memory>
#include <string>

#include "Image2D.h"

// ImageResourceId：
// ResourceManager 中通用图片资源的索引 ID。
// 外部系统通过这个 ID 获取 Image2D，而不是直接持有图片路径。
enum ImageResourceId
{
    IMG_BG_SKY,
    IMG_BG_CLOUDS,
    IMG_BG_FLORA1,
    IMG_BG_FLORA2,

    IMG_TILESET_MAIN,

    IMG_PLAYER_IDLE_L,
    IMG_PLAYER_IDLE_R,
    IMG_PLAYER_WALK_L,
    IMG_PLAYER_WALK_R,
    IMG_PLAYER_RUN_L,
    IMG_PLAYER_RUN_R,
    IMG_PLAYER_JUMP_START_L,
    IMG_PLAYER_JUMP_START_R,
    IMG_PLAYER_JUMP_LOOP_L,
    IMG_PLAYER_JUMP_LOOP_R,
    IMG_PLAYER_JUMP_END_L,
    IMG_PLAYER_JUMP_END_R,

    IMG_PLAYER2_STATIC,
    IMG_PLAYER3_STATIC,
    IMG_PLAYER4_STATIC,

    IMG_COIN_GOLD,
    IMG_COIN_SILVER,
    IMG_COIN_COPPER,



    IMG_COIN_COLLECTED,

    IMG_CHECKPOINT_NO_FLAG,
    IMG_CHECKPOINT_FLAG_OUT,
    IMG_CHECKPOINT_FLAG_IDLE,

	IMAGE_ENDPOINT_IDLE,
	IMAGE_ENDPOINT_PRESSED,

    IMG_APPLE,
    IMG_BANANA,
    IMG_MELON,
    IMG_ORANGE,
    IMG_PINEAPPLE,
    IMG_STRAWBERRY,
    IMG_KIWI,
    IMG_CHERRY,
    IMG_RESOURCE_COUNT
};

// TextResourceId：
// ResourceManager 中通用文本/数据资源的索引 ID。
enum TextResourceId
{
    TXT_MAP_MAIN,

    TXT_RESOURCE_COUNT
};

// ResourceManager：
// 当前关卡图片及数据资源管理器。
// 它根据 ImageResourceId 和 TextResourceId 注册和加载图片与文本资源。
class ResourceManager
{
private:
    // 注册表：存储“图片资源 ID”到“图片相对路径”的字典映射。
    // 使用 std::map 能够在加载资源前，先建立清晰的文件定位关联。
    map<ImageResourceId, basic_string<TCHAR>> imagePaths;

    // 缓存表：存储“图片资源 ID”到“智能指针所托管的 Image2D 图像对象”的映射。
    // 用 unique_ptr 智能指针管理动态生成的对象，它对持有的堆内存具有绝对所有权（Exclusive ownership）。
    // 在 map 被 clear 或是 ResourceManager 销毁时，所有被智能指针托管的对象都会被自动释放，安全无痛。
    map<ImageResourceId, unique_ptr<Image2D>> images;

    // 注册表：存储“文本资源 ID”到“文本相对路径”的映射。
    map<TextResourceId, basic_string<TCHAR>> textPaths;

    // 缓存表：存储“文本资源 ID”到“已读取的文本具体内容（std::string）”的映射。
    map<TextResourceId, string> textContents;

public:
    // 功能：根据图片资源 ID 获取 EasyX 图片指针。
    IMAGE* getRawImage(ImageResourceId id);

    // 功能：注册当前关卡需要的通用图片资源路径。
    void initImageResourceTable();

    // 功能：根据图片资源 ID 加载一张 Image2D，并保存到资源表。
    void loadImage2D(ImageResourceId id);

    // 功能：加载当前资源表中注册的所有通用图片资源。
    void loadImageResources();

    // 功能：根据图片资源 ID 获取已加载的 Image2D。
    Image2D* getImage2D(ImageResourceId id);

    // 功能：注册当前关卡需要的通用文本资源路径。
    void initTextResourceTable();

    // 功能：根据文本资源 ID 加载文本文件，并保存到缓存表。
    void loadTextFile(TextResourceId id);

    // 功能：加载当前资源表中注册的所有文本资源。
    void loadTextResources();

    // 功能：根据文本资源 ID 获取已加载的文本内容。
    const string& getTextContent(TextResourceId id) const;

    // 功能：加载当前关卡需要的所有资源（图片和文本）。
    void loadLevelResources();
};