#include "Resource.h"
// 引入 nlohmann/json 第三方库，用于解析游戏资源定义配置文件 assets.json，
// 以便读取不同图片与文本素材的相对物理存储路径。
#include "json.hpp"



IMAGE* ResourceManager::getRawImage(const std::string& name)
{
    Image2D* image = getImage2D(name);

    if (image == NULL)
    {
        return NULL;
    }

    return image->getImage();
}

void ResourceManager::initImageResourceTable()
{
    imagePaths.clear();

    std::ifstream f("assets/data/assets.json");
    if (!f.is_open())
    {
        std::cout << "Failed to open assets.json" << endl;
        return;
    }

    // 声明并解析 JSON 模板类对象。它读取流中的 assets.json 整体结构，
    // 构建出包含 images 和 texts 数组的关联配置树。
    nlohmann::json data;
    f >> data;
    f.close();

    for (auto& item : data["images"])
    {
        std::string nameStr = item["name"];
        std::string pathStr = item["path"];
        std::basic_string<TCHAR> path(pathStr.begin(), pathStr.end());
        // 使用 std::unordered_map，以平均 O(1) 的哈希开销，
        // 建立“图片资源名称（Name）”与“图片物理文件相对路径”的映射。
        imagePaths[nameStr] = path;
    }
}

void ResourceManager::loadImage2D(const std::string& name)
{
    // 在无序哈希表中查找路径，确保此图片已被注册。
    if (imagePaths.find(name) == imagePaths.end())
    {
        return;
    }

    // 实例化 std::unique_ptr 智能指针管理动态生成的 Image2D 图像对象。
    std::unique_ptr<Image2D> image(new Image2D());

    if (!image->load(imagePaths[name].c_str()))
    {
        return;
    }

    // 通过 std::move 将智能指针的所有权（Ownership）转移给 `images` 字典。
    images[name] = std::move(image);
}

void ResourceManager::loadImageResources()
{
    // 清空缓存表并自动释放之前加载的图像内存。
    images.clear();

    // 遍历注册好的图片相对路径表，逐一加载。
    for (auto& pair : imagePaths)
    {
        loadImage2D(pair.first);
    }
}

Image2D* ResourceManager::getImage2D(const std::string& name)
{
    // 平均 O(1) 快速查找该图片是否已经成功加载。
    if (images.find(name) == images.end())
    {
        return NULL;
    }

    // 返回内部托管的原始 naked 指针供外部使用，ResourceManager 仍负责其生命周期。
    return images[name].get();
}

void ResourceManager::initTextResourceTable()
{
    textPaths.clear();

    std::ifstream f("assets/data/assets.json");
    if (!f.is_open())
    {
        std::cout << "Failed to open assets.json" << endl;
        return;
    }

    nlohmann::json data;
    f >> data;
    f.close();

    for (auto& item : data["texts"])
    {
        std::string nameStr = item["name"];
        std::string pathStr = item["path"];
        std::basic_string<TCHAR> path(pathStr.begin(), pathStr.end());
        textPaths[nameStr] = path;
    }
}

void ResourceManager::loadTextFile(const std::string& name)
{
    if (textPaths.find(name) == textPaths.end())
    {
        return;
    }

    std::ifstream inFile(textPaths[name].c_str());
    if (!inFile.is_open())
    {
        std::cout << "Failed to open text resource file." << endl;
        return;
    }

    std::string content = "";
    std::string line;
    while (std::getline(inFile, line))
    {
        content += line + "\n";
    }
    textContents[name] = content;
    inFile.close();
}

void ResourceManager::loadTextResources()
{
    textContents.clear();
    for (auto& pair : textPaths)
    {
        loadTextFile(pair.first);
    }
}

const std::string& ResourceManager::getTextContent(const std::string& name) const
{
    static const std::string emptyString = "";
    auto it = textContents.find(name);
    if (it == textContents.end())
    {
        return emptyString;
    }
    return it->second;
}

void ResourceManager::loadLevelResources()
{
    initImageResourceTable();
    loadImageResources();

    initTextResourceTable();
    loadTextResources();
}