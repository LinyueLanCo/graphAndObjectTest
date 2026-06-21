#include "DialogueBox.h"
#include "Renderer.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <cctype>

// 5行10列的字集图对照字符串映射定义
const std::string FONT_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ    0123456789.,:?!()+- ";

DialogueBox::DialogueBox() : UIElement()
{
    fullText = "";
    displayText = "";
    textProgress = 0.0;
    readSpeed = 0.35; // 默认打字速度（每帧增加0.35个字，约每秒21个字）
    isFinished = true;
    fontTexture = nullptr;

    charWidth = 8;
    charHeight = 10;
    charScale = 2.0;   // 默认放大两倍，显示为 16x20 像素的清晰字符
    charSpacing = 2;
    lineSpacing = 8;
    paddingLeft = 30;
    paddingTop = 25;
}

void DialogueBox::initDialogue(IMAGE* newFontTexture)
{
    fontTexture = newFontTexture;
    
    // 设置对话框大小为宽度 560，高度 110 像素
    int boxW = 560;
    int boxH = 110;
    
    // 初始化父类 UIElement：屏幕底端水平居中对齐，距离屏幕底边缘 140 像素作为目标位置
    this->init(boxW, boxH, UI_CENTER, 0, 140);
}

void DialogueBox::startDialogue(const std::string& text)
{
    fullText = text;
    
    // 强制把所有输入字符转换为大写，因为贴图上只有大写字母
    std::transform(fullText.begin(), fullText.end(), fullText.begin(), [](unsigned char c) {
        return std::toupper(c);
    });

    displayText = "";
    textProgress = 0.0;
    isFinished = false;
}

void DialogueBox::showDialogueWithOffset(double offsetX, double offsetY)
{
    this->refreshTarget();
    double tx = this->getTargetX();
    double ty = this->getTargetY();
    this->setPosition(tx + offsetX, ty + offsetY);
    this->showAnimated();
}

void DialogueBox::hideDialogueWithOffset(double offsetX, double offsetY)
{
    this->refreshTarget();
    double tx = this->getTargetX();
    double ty = this->getTargetY();
    this->setTargetPosition(tx + offsetX, ty + offsetY);
    this->hideAnimated();
}

void DialogueBox::advance()
{
    if (!isFinished)
    {
        // 如果打字没结束，直接拉满进度，瞬间显示完整文本
        textProgress = (double)fullText.length();
        displayText = fullText;
        isFinished = true;
    }
    else
    {
        // 如果打字已结束，向下滑动收回隐藏对话框 (偏移量设为 200 像素)
        this->hideDialogueWithOffset(0, 200);
    }
}

void DialogueBox::updateDialogue()
{
    if (!this->isActive() || isFinished)
    {
        return;
    }

    textProgress += readSpeed;
    if (textProgress >= (double)fullText.length())
    {
        textProgress = (double)fullText.length();
        isFinished = true;
    }

    int visibleCount = (int)textProgress;
    displayText = fullText.substr(0, visibleCount);
}

void DialogueBox::draw(Renderer& renderer)
{
    if (!this->isVisible())
    {
        return;
    }

    // 1. 绘制对话框底座圆角面板背景（白色背景加灰色边框）
    renderer.drawUIElementPanel(*this);

    if (fontTexture == nullptr || displayText.empty())
    {
        return;
    }

    // 2. 逐字换行排版并直接调用公开化的底层贴图切片绘制
    UIBox box = this->getBox();
    
    int drawX = box.x + paddingLeft;
    int drawY = box.y + paddingTop;
    
    int drawCharW = (int)(charWidth * charScale);
    int drawCharH = (int)(charHeight * charScale);

    // 最大可绘制宽度限制，超过此坐标即自动换行
    int maxRight = box.x + box.w - paddingLeft;

    for (char c : displayText)
    {
        if (c == '\n')
        {
            // 显式换行
            drawX = box.x + paddingLeft;
            drawY += drawCharH + lineSpacing;
            continue;
        }

        if (c == ' ')
        {
            // 空格不绘制图像，直接横坐标步进
            drawX += drawCharW + charSpacing;
            
            // 自动折行检测
            if (drawX + drawCharW > maxRight)
            {
                drawX = box.x + paddingLeft;
                drawY += drawCharH + lineSpacing;
            }
            continue;
        }

        // 查找字符在映射对照串中的索引
        size_t found = FONT_CHARS.find(c);
        if (found != std::string::npos)
        {
            int index = (int)found;
            int col = index % 10;
            int row = index / 10;
            int srcX = col * charWidth;
            int srcY = row * charHeight;

            // 统一调用已经公有化的 drawImageTileAlpha 完成 Alpha 混合透明像素绘制
            renderer.drawImageTileAlpha(
                drawX,
                drawY,
                drawCharW,
                drawCharH,
                fontTexture,
                srcX,
                srcY,
                charWidth,
                charHeight
            );
        }

        // 步进到下一个字位置
        drawX += drawCharW + charSpacing;

        // 自动换行检查：如果下一个字的位置超出了对话框右边缘，则提前换行
        if (drawX + drawCharW > maxRight)
        {
            drawX = box.x + paddingLeft;
            drawY += drawCharH + lineSpacing;
        }
    }
}
