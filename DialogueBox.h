#pragma once

#include "UI.h"
#include <string>

class Image2D;

// DialogueConfig：对话框样式与打字机配置结构体
struct DialogueConfig
{
    double readSpeed = 0.35;               // 打字速度（字符数/帧）
    double charScale = 2.0;                // 字符缩放倍数
    int boxW = 560;                        // 对话框宽度
    int boxH = 98;                         // 对话框高度（刚好两行）
    int marginY = 140;                     // 距离底部的边距
    int paddingLeft = 30;                  // 文字排版左边距
    int paddingTop = 25;                   // 文字排版顶边距
    int charSpacing = 2;                   // 字符间距
    int lineSpacing = 8;                   // 行间距
    int charWidth = 8;                     // 单个字符原图宽度 (8)
    int charHeight = 10;                    // 单个字符原图高度 (10)
    COLORREF bgColor = RGB(20, 24, 40);    // 对话框背景色（深蓝/黑色）
    COLORREF borderColor = RGB(255, 255, 255); // 对话框边线色（白色）
    bool autoClose = false;                // 是否启用自动关闭
    double autoCloseDuration = 180.0;      // 自动关闭的倒计时帧数（约 3 秒）

    static DialogueConfig Default()
    {
        return DialogueConfig();
    }
};

// DialogueBox：
// 独立对话框组件，继承自 UIElement，具备滑入滑出能力，专职处理打字机和像素字体逐字绘制。
class DialogueBox : public UIElement
{
private:
    std::string fullText;       // 完整的大写文本内容
    std::string displayText;    // 当前帧截取出来的已打字完成的文本内容
    
    double textProgress;        // 字符裁剪进度计数器（每帧增加）
    bool isFinished;            // 是否已打完当前页的所有文字

    Image2D* fontTexture;         // 指向 8x10.png 像素白字体大图的指针
    
    DialogueConfig config;      // 当前正在使用的对话框配置

    bool isAutoCloseEnabled;    // 是否启用了自动收回
    double autoCloseTimer;      // 自动收回倒计时剩余帧数

    // 预计算文字排版所需的总高度
    double calculateRequiredHeight(const std::string& text) const;

public:
    DialogueBox();

    // 初始化对话框基本属性并绑定字体贴图
    void initDialogue(Image2D* newFontTexture, const DialogueConfig& newConfig = DialogueConfig::Default());

    // 载入新文本，重置打字机所有状态，并可按需提供覆盖配置
    void startDialogue(const std::string& text, const DialogueConfig& newConfig = DialogueConfig::Default());

    // 触发显示与滑动效果（直接操作实例，规避对象切片）
    void showDialogueWithOffset(double offsetX, double offsetY);
    void hideDialogueWithOffset(double offsetX, double offsetY);

    const std::string& getFullText() const { return fullText; }
    const DialogueConfig& getConfig() const { return config; }
    Image2D* getFontTexture() const { return fontTexture; }
    const std::string& getDisplayText() const { return displayText; }

    // 对话交互推进接口：
    // 如果打字没结束则瞬间全显示；如果已经打完字，则进行滑下收回动画
    void advance();

    // 每帧被关卡调用，推进打字机字符进度
    void updateDialogue();

    // 获取当前打字是否已全部结束
    bool isTypewriterFinished() const { return isFinished; }
};

