#include "Resource.h"
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

    nlohmann::json data;
    f >> data;
    f.close();

    for (auto& item : data["images"])
    {
        int idVal = item["id"];
        string pathStr = item["path"];
        basic_string<TCHAR> path(pathStr.begin(), pathStr.end());
        imagePaths[(ImageResourceId)idVal] = path;
    }
}

void ResourceManager::loadImage2D(ImageResourceId id)
{
    if (imagePaths.find(id) == imagePaths.end())
    {
        return;
    }

    unique_ptr<Image2D> image(new Image2D());

    if (!image->load(imagePaths[id].c_str()))
    {
        return;
    }

    images[id] = move(image);
}

void ResourceManager::loadImageResources()
{
    images.clear();

    for (map<ImageResourceId, basic_string<TCHAR>>::iterator it = imagePaths.begin(); it != imagePaths.end(); ++it)
    {
        loadImage2D(it->first);
    }
}

Image2D* ResourceManager::getImage2D(ImageResourceId id)
{
    if (images.find(id) == images.end())
    {
        return NULL;
    }

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

    nlohmann::json data;
    f >> data;
    f.close();

    for (auto& item : data["texts"])
    {
        int idVal = item["id"];
        string pathStr = item["path"];
        basic_string<TCHAR> path(pathStr.begin(), pathStr.end());
        textPaths[(TextResourceId)idVal] = path;
    }
}

void ResourceManager::loadTextFile(TextResourceId id)
{
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
    textContents[id] = content;
    inFile.close();
}

void ResourceManager::loadTextResources()
{
    textContents.clear();
    for (map<TextResourceId, basic_string<TCHAR>>::iterator it = textPaths.begin(); it != textPaths.end(); ++it)
    {
        loadTextFile(it->first);
    }
}

const string& ResourceManager::getTextContent(TextResourceId id) const
{
    static const string emptyString = "";
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