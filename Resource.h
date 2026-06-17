#pragma once
#include "Config.h"

#include "Image2D.h"
#include <string>

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
    map<ImageResourceId, const TCHAR*> imagePaths;
    map<ImageResourceId, unique_ptr<Image2D>> images;

    map<TextResourceId, const TCHAR*> textPaths;
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