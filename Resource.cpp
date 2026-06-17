#include "Resource.h"
// 引入 nlohmann/json 第三方库，用于解析游戏资源定义配置文件 assets.json，
// 以便读取不同图片与文本素材的相对物理存储路径。
#include "json.hpp"



IMAGE* ResourceManager::getRawImage(ImageResourceId id)
{
    Image2D* image = getImage2D(id);

    if (image == NULL)
    {
        return NULL;
    }

    return image->getImage();
}

void ResourceManager::initImageResourceTable()
{
    imagePaths.clear();

    ifstream f("assets/data/assets.json");
    if (!f.is_open())
    {
        cout << "Failed to open assets.json" << endl;
        return;
    }

    // 声明并解析 JSON 模板类对象。它读取流中的 assets.json 整体结构，
    // 构建出包含 images 和 texts 数组的关联配置树。
    nlohmann::json data;
    f >> data;
    f.close();

    for (auto& item : data["images"])
    {
        int idVal = item["id"];
        string pathStr = item["path"];
        basic_string<TCHAR> path(pathStr.begin(), pathStr.end());
        // 使用 std::map::operator[]，以稳定的 O(log N) 二叉树插入开销，
        // 建立“图片资源 ID（ImageResourceId）”与“图片物理文件相对路径”的字典映射。
        imagePaths[(ImageResourceId)idVal] = path;
    }
}

void ResourceManager::loadImage2D(ImageResourceId id)
{
    // 调用 std::map::find 检索资源相对路径。
    // 它利用红黑树的对数查找（时间复杂度为 O(log N)），快速判定此图片资源 ID 是否已被注册。
    if (imagePaths.find(id) == imagePaths.end())
    {
        return;
    }

    // 实例化 std::unique_ptr 智能指针管理动态生成的 Image2D 图像对象。
    // 使用智能指针主要是为了内存安全——如果中途图片解析失败 return，或者发生异常，
    // C++ 的 RAII 机制会保证该局部智能指针析构时自动释放申请的堆内存，从而避免内存泄漏。
    unique_ptr<Image2D> image(new Image2D());

    if (!image->load(imagePaths[id].c_str()))
    {
        return;
    }

    // 通过 std::move 将智能指针的独占控制权（Ownership）转移给 `images` 字典。
    // 因为 unique_ptr 是不允许拷贝的（Copy disabled），必须使用移动语义将指针所有权塞入 map 容器。
    // 此后，原来的局部变量 image 将不再拥有该对象，其内部存的值会变为空指针。
    images[id] = move(image);
}

void ResourceManager::loadImageResources()
{
    // 清空缓存表。
    // 调用 std::map::clear() 除了注销键值对节点外，还会触发其中所有 unique_ptr 的析构函数，
    // 从而自动安全释放之前加载的全部 Image2D 图像占用的内存。
    images.clear();

    // 使用 std::map::iterator 迭代器来遍历注册好的图片相对路径表。
    // 红黑树是有序的，所以迭代器会按照 Key（ImageResourceId 枚举值）递增的顺序自动遍历。
    for (map<ImageResourceId, basic_string<TCHAR>>::iterator it = imagePaths.begin(); it != imagePaths.end(); ++it)
    {
        loadImage2D(it->first);
    }
}

Image2D* ResourceManager::getImage2D(ImageResourceId id)
{
    // O(log N) 快速查找该图片是否已经成功加载到 images 缓存中
    if (images.find(id) == images.end())
    {
        return NULL;
    }

    // 调用 std::unique_ptr::get() 获取内部托管的原始 naked 指针。
    // 这样做只是把访问权限借给外部使用，ResourceManager 仍然负责该对象的生命周期（拥有所有权），
    // 外部调用者拿到此指针进行渲染即可，千万不要擅自 delete 释放它！
    return images[id].get();
}

void ResourceManager::initTextResourceTable()
{
    textPaths.clear();

    ifstream f("assets/data/assets.json");
    if (!f.is_open())
    {
        cout << "Failed to open assets.json" << endl;
        return;
    }

    // 同样通过 json 模板类解析读取文本资源（主要是关卡地图的 txt 路径）
    nlohmann::json data;
    f >> data;
    f.close();

    for (auto& item : data["texts"])
    {
        int idVal = item["id"];
        string pathStr = item["path"];
        basic_string<TCHAR> path(pathStr.begin(), pathStr.end());
        // 使用 std::map::operator[] 以 O(log N) 的复杂度将文本资源路径关联到字典中
        textPaths[(TextResourceId)idVal] = path;
    }
}

void ResourceManager::loadTextFile(TextResourceId id)
{
    // O(log N) 检索文本相对路径表，确保传入的 TextResourceId 存在
    if (textPaths.find(id) == textPaths.end())
    {
        return;
    }

    ifstream inFile(textPaths[id].c_str());
    if (!inFile.is_open())
    {
        cout << "Failed to open text resource file." << endl;
        return;
    }

    string content = "";
    string line;
    while (getline(inFile, line))
    {
        content += line + "\n";
    }
    // 将读取完成的整个文件文本存入 textContents 字典中
    textContents[id] = content;
    inFile.close();
}

void ResourceManager::loadTextResources()
{
    textContents.clear();
    // 使用 std::map::iterator 迭代器，按 Key（TextResourceId）大小顺序依次加载对应的文本内容
    for (map<TextResourceId, basic_string<TCHAR>>::iterator it = textPaths.begin(); it != textPaths.end(); ++it)
    {
        loadTextFile(it->first);
    }
}

const string& ResourceManager::getTextContent(TextResourceId id) const
{
    static const string emptyString = "";
    // 用 map::const_iterator 迭代器以 O(log N) 复杂度只读检索对应的文本缓存
    map<TextResourceId, string>::const_iterator it = textContents.find(id);
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