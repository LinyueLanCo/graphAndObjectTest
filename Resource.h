#pragma once
#include "Config.h"

// Image2D：
// 包装一张 EasyX IMAGE，并记录图片基础信息。
// 它只负责加载和提供图片资源，不负责绘制、不负责动画、不负责坐标。
class Image2D
{
private:
    IMAGE image;

    int width;
    int height;

public:
    // 功能：初始化一个空图片资源。
    Image2D();

    // 功能：从文件加载图片，并记录图片宽高。
    bool load(const TCHAR* path);

    // 功能：从文件加载图片，并按指定大小缩放到内存图片。
    bool load(const TCHAR* path, int loadW, int loadH);

    // 功能：获取 EasyX 原始图片指针，供底层绘制函数使用。
    IMAGE* getImage();

    // 功能：获取图片宽度。
    int getWidth() const;

    // 功能：获取图片高度。
    int getHeight() const;
};

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

    IMG_RESOURCE_COUNT
};

// ResourceManager：
// 当前关卡图片资源管理器。
// 它根据 ImageResourceId 注册和加载 Image2D，并提供稳定的图片资源查询接口。
// 它不负责动画片段、地图 tile 规则、背景对象逻辑和绘制逻辑。
class ResourceManager
{
private:
    map<ImageResourceId, const TCHAR*> imagePaths;
    map<ImageResourceId, unique_ptr<Image2D>> images;

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

    // 功能：加载当前关卡需要的图片资源。
    void loadLevelResources();
};