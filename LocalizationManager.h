#pragma once

#include <string>
#include <unordered_map>

// LocalizationManager：
// 本地化文本管理器，负责解析外部 JSON 文件并提供只读翻译匹配接口。
class LocalizationManager
{
private:
    // std::unordered_map 是标准模板库（STL）中的哈希表关联容器。
    // 这里用于快速基于 Key 字符串查询对应的中文或英文文本 Value 字符串。
    std::unordered_map<std::string, std::string> stringTable;

public:
    LocalizationManager();

    // 加载并解析 JSON 文本配置文件
    bool loadLanguage(const std::string& filepath);

    // 根据 Key 获取对应的大写文本内容。若找不到则返回带格式的友情错误字符串。
    std::string getString(const std::string& key) const;

    // 清理管理器缓存
    void clear();
};
