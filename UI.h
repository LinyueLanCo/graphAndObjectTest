#pragma once

#include "Config.h"

// UIAnchor：
// UI 锚点枚举。用于把 UIBox 放置到窗口四角或中心。
enum UIAnchor
{
    UI_TOP_LEFT,
    UI_TOP_RIGHT,
    UI_BOTTOM_LEFT,
    UI_BOTTOM_RIGHT,
    UI_CENTER
};

// UIBox：
// 屏幕空间下的 UI 矩形框，x/y 是左上角，w/h 是宽高。
struct UIBox
{
    int x;
    int y;
    int w;
    int h;
};

// UIElementState：
// UI 元素生命周期状态。用于区分隐藏、进入、显示和退出。
enum UIElementState
{
    UI_HIDDEN,
    UI_SHOWING,
    UI_VISIBLE,
    UI_HIDING
};

UIBox makeViewportUIBox();
UIBox makeUIBoxByParentAnchor(
    UIBox parent,
    int w,
    int h,
    UIAnchor anchor,
    int marginX,
    int marginY
);
UIBox makeUIBoxByAnchor(
    int w,
    int h,
    UIAnchor anchor,
    int marginX,
    int marginY
);
void drawUIBox(UIBox box, COLORREF fillColor, COLORREF borderColor);

// UIElement：
// 通用 UI 元素基础类，只使用屏幕坐标 / 父级 UI 坐标，不依赖世界坐标和 Camera。
class UIElement
{
private:
    double x;
    double y;

    double targetX;
    double targetY;

    int w;
    int h;

    UIAnchor anchor;
    int marginX;
    int marginY;

    int parentIndex;

    bool active;
    bool visible;
    bool interactable;

    double moveSpeed;

    UIElementState state;

public:
    UIElement();

    void snapToTarget();

    void setParentIndex(int newParentIndex);
    int getParentIndex() const;

    void setPosition(double newX, double newY);
    void setTargetPosition(double newTargetX, double newTargetY);
    double getTargetX() const;
    double getTargetY() const;

    void init(int newW, int newH, UIAnchor newAnchor, int newMarginX, int newMarginY);
    void refreshTarget();
    void setAnchor(UIAnchor newAnchor);
    void refreshTargetByParentBox(UIBox parentBox);

    void showInstant();
    void showAnimated();
    void show();
    void hide();
    void hideAnimated();

    void update();

    UIBox getBox() const;
    bool isActive() const;
    bool isVisible() const;
    bool isInteractable() const;
    UIElementState getState() const;
};

// UIManager：
// 管理当前界面中的 UIElement，负责父级关系刷新和统一 update。
class UIManager
{
private:
    vector<UIElement> elements;

public:
    int addElement(UIElement element);

    bool isValidIndex(int index) const;
    bool isElementEffectivelyVisible(int index) const;

    UIElement& getElement(int index);
    const UIElement& getElement(int index) const;
    UIBox getElementBox(int index) const;

    void refreshElementTarget(int index);
    void showElementInstant(int index);
    void showElementAnimated(int index);
    void showElementAnimatedFromOffset(int index, double offsetX, double offsetY);
    void hideElementAnimatedToOffset(int index, double offsetX, double offsetY);
    void hideElementInstant(int index);

    void update();
};
