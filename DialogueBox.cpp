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
    isFinished = true;
    fontTexture = nullptr;
    config = DialogueConfig::Default();
    isAutoCloseEnabled = false;
    autoCloseTimer = 0.0;
}

void DialogueBox::initDialogue(IMAGE* newFontTexture, const DialogueConfig& newConfig)
{
    fontTexture = newFontTexture;
    config = newConfig;
    isAutoCloseEnabled = false;
    autoCloseTimer = 0.0;
    
    // 初始化父类 UIElement：屏幕底端水平居中对齐，距离屏幕底边缘 marginY 像素作为目标位置
    this->init(config.boxW, config.boxH, UI_CENTER, 0, config.marginY);
    
    // 初始状态强制隐藏，且将当前位置设为完全在屏幕下边界（900 像素）之外 (即 targetY + 400)
    this->hide();
    this->refreshTarget();
    this->setPosition(this->getTargetX(), this->getTargetY() + 400.0);
}

void DialogueBox::startDialogue(const std::string& text, const DialogueConfig& newConfig)
{
    config = newConfig;
    isAutoCloseEnabled = config.autoClose;
    autoCloseTimer = config.autoCloseDuration;

    fullText = text;
    
    // 强制把所有输入字符转换为大写，因为贴图上只有大写字母
    std::transform(fullText.begin(), fullText.end(), fullText.begin(), [](unsigned char c) {
        return std::toupper(c);
    });

    // 1. 重新根据配置初始化 UIElement 的位置与大小（以默认 boxH，即两行高起步）
    this->init(config.boxW, config.boxH, UI_CENTER, 0, config.marginY);
    // 保持隐藏状态，初始置于窗口下边界之外
    this->hide();
    this->refreshTarget();
    this->setPosition(this->getTargetX(), this->getTargetY() + 400.0);

    // 2. 将初始目标高度设为起步高度，不在此处提前拉伸
    this->setTargetSize(config.boxW, config.boxH);

    displayText = "";
    textProgress = 0.0;
    isFinished = false;
}

void DialogueBox::showDialogueWithOffset(double offsetX, double offsetY)
{
    this->refreshTarget();
    double tx = this->getTargetX();
    double ty = this->getTargetY();
    
    // 固定的 400 像素偏移量
    double finalOffsetY = (offsetY > 0) ? 400.0 : offsetY;
    
    this->setPosition(tx + offsetX, ty + finalOffsetY);
    this->showAnimated();
}

void DialogueBox::hideDialogueWithOffset(double offsetX, double offsetY)
{
    // 统一收回入口：一旦执行收回，立即重置并注销自动关闭状态
    isAutoCloseEnabled = false;
    autoCloseTimer = 0.0;

    this->refreshTarget();
    double tx = this->getTargetX();
    double ty = this->getTargetY();
    
    double finalOffsetY = (offsetY > 0) ? 400.0 : offsetY;
    
    this->setTargetPosition(tx + offsetX, ty + finalOffsetY);
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

        // 瞬间跳过时，利用提前计算高度将目标高度拉伸至完整文本所需高度，从而正常触发高度平滑拉伸
        double neededH = calculateRequiredHeight(fullText);
        this->setTargetSize(config.boxW, neededH);
    }
    else
    {
        // 如果打字已结束，向下滑动收回隐藏对话框 (使用 400 像素偏移)
        this->hideDialogueWithOffset(0, 400);
    }
}

void DialogueBox::updateDialogue()
{
    if (!this->isActive())
    {
        return;
    }

    // 1. 如果打字机未播完，推进打字机文本截取进度
    if (!isFinished)
    {
        textProgress += config.readSpeed;
        if (textProgress >= (double)fullText.length())
        {
            textProgress = (double)fullText.length();
            isFinished = true;
        }

        int visibleCount = (int)textProgress;
        displayText = fullText.substr(0, visibleCount);

        // 实时根据当前已显示的文字计算所需的高度，并动态设置目标高度以触发平滑插值拉伸
        double currentH = calculateRequiredHeight(displayText);
        this->setTargetSize(config.boxW, currentH);
    }

    // 2. 如果打字全部完成，且配置了自动收回，则更新倒计时并在到期后收回
    if (isFinished && isAutoCloseEnabled)
    {
        autoCloseTimer -= 1.0;
        if (autoCloseTimer <= 0.0)
        {
            this->hideDialogueWithOffset(0, 400);
        }
    }
}

double DialogueBox::calculateRequiredHeight(const std::string& text) const
{
    // 模拟排版换行过程
    int lines = 1;
    int drawCharW = (int)(config.charWidth * config.charScale);
    int drawCharH = (int)(config.charHeight * config.charScale);
    int maxRight = config.boxW - config.paddingLeft;
    int drawX = config.paddingLeft;

    // 5行10列的字集图对照字符串映射定义
    static const std::string FONT_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ    0123456789.,:?!()+- ";

    for (char c : text)
    {
        if (c == '\n')
        {
            drawX = config.paddingLeft;
            lines++;
            continue;
        }

        if (c == ' ')
        {
            drawX += drawCharW + config.charSpacing;
            if (drawX + drawCharW > maxRight)
            {
                drawX = config.paddingLeft;
                lines++;
            }
            continue;
        }

        // 步进到下一个字位置
        drawX += drawCharW + config.charSpacing;

        // 自动换行检查
        if (drawX + drawCharW > maxRight)
        {
            drawX = config.paddingLeft;
            lines++;
        }
    }

    // 理想高度 = paddingTop + paddingBottom + 行数 * 字符高 + (行数 - 1) * 行间距
    // 我们假设 paddingBottom 和 paddingTop 一致，均使用 paddingTop
    double neededH = config.paddingTop + config.paddingTop + lines * drawCharH;
    if (lines > 1)
    {
        neededH += (lines - 1) * config.lineSpacing;
    }

    // 限制最小高度为配置的 boxH，防止过短的文本使对话框缩得太小
    if (neededH < config.boxH)
    {
        neededH = config.boxH;
    }

    return neededH;
}


