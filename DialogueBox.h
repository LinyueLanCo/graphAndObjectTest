#pragma once

#include "UI.h"
#include <string>

// DialogueBox：
// 独立对话框组件，继承自 UIElement，具备滑入滑出能力，专职处理打字机和像素字体逐字绘制。
class DialogueBox : public UIElement
{
private:
    std::string fullText;       // 完整的大写文本内容
    std::string displayText;    // 当前帧截取出来的已打字完成的文本内容
    
    double textProgress;        // 字符裁剪进度计数器（每帧增加）
    double readSpeed;           // 打字速度（字符数/帧）
    bool isFinished;            // 是否已打完当前页的所有文字

    IMAGE* fontTexture;         // 指向 8x10.png 像素白字体大图的指针
    
    int charWidth;              // 单个字符原图宽度 (8)
    int charHeight;             // 单个字符原图高度 (10)
    double charScale;           // 绘制时的放大缩放倍数 (2.0)
    int charSpacing;            // 字符之间的水平间距 (2)
    int lineSpacing;            // 换行时的垂直行间距 (6)
    int paddingLeft;            // 文字排版左侧内边距 (30)
    int paddingTop;             // 文字排版顶部内边距 (25)

public:
    DialogueBox();

    // 初始化对话框基本属性并绑定字体贴图
    void initDialogue(IMAGE* newFontTexture);

    // 载入新文本，重置打字机所有状态
    void startDialogue(const std::string& text);

    // 触发显示与滑动效果（直接操作实例，规避对象切片）
    void showDialogueWithOffset(double offsetX, double offsetY);
    void hideDialogueWithOffset(double offsetX, double offsetY);

    // 对话交互推进接口：
    // 如果打字没结束则瞬间全显示；如果已经打完字，则进行滑下收回动画
    void advance();

    // 每帧被关卡调用，推进打字机字符进度
    void updateDialogue();

    // 绘制对话框底图框架，并在其内对已截取的 displayText 进行逐字换行排版透明渲染
    void draw(class Renderer& renderer);

    // 获取当前打字是否已全部结束
    bool isTypewriterFinished() const { return isFinished; }
};
