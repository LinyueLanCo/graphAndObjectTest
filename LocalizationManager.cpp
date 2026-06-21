#include "LocalizationManager.h"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

LocalizationManager::LocalizationManager()
{
}

bool LocalizationManager::loadLanguage(const std::string& filepath)
{
    std::ifstream f(filepath);
    if (!f.is_open())
    {
        std::cout << "无法打开本地化文本配置文件: " << filepath << std::endl;
        return false;
    }

    nlohmann::json data;
    try {
        f >> data;
        f.close();
    }
    catch (const std::exception& e) {
        std::cout << "解析本地化文本 JSON 出错: " << e.what() << std::endl;
        f.close();
        return false;
    }

    stringTable.clear();

    for (auto& item : data.items())
    {
        std::string key = item.key();
        std::string val = item.value().get<std::string>();

        // 统一转换为大写，因为像素字体贴图仅包含大写字母
        std::transform(val.begin(), val.end(), val.begin(), [](unsigned char c) {
            return std::toupper(c);
        });

        stringTable[key] = val;
    }

    std::cout << "本地化语言表加载完毕，共从 \"" << filepath << "\" 加载了 " << stringTable.size() << " 条文本。" << std::endl;
    return true;
}

std::string LocalizationManager::getString(const std::string& key) const
{
    auto it = stringTable.find(key);
    if (it != stringTable.end())
    {
        return it->second;
    }
    return "[MISSING KEY: " + key + "]";
}

void LocalizationManager::clear()
{
    stringTable.clear();
}
