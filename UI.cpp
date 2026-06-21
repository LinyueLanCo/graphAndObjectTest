#include "UI.h"
#include "MathUtils.h"

// 功能：获取当前窗口对应的顶层 UI 区域。
UIBox makeViewportUIBox()
{
    UIBox box;

    box.x = 0;
    box.y = 0;
    box.w = WINDOW_WIDTH;
    box.h = WINDOW_HEIGHT;

    return box;
}

// 功能：根据父级 UIBox、锚点和 margin 计算子 UI 的屏幕位置。
UIBox makeUIBoxByParentAnchor(
    UIBox parent,
    int w,
    int h,
    UIAnchor anchor,
    int marginX,
    int marginY
)
{
    UIBox box;

    box.w = w;
    box.h = h;

    if (anchor == UI_TOP_LEFT)
    {
        box.x = parent.x + marginX;
        box.y = parent.y + marginY;
    }
    else if (anchor == UI_TOP_RIGHT)
    {
        box.x = parent.x + parent.w - w - marginX;
        box.y = parent.y + marginY;
    }
    else if (anchor == UI_BOTTOM_LEFT)
    {
        box.x = parent.x + marginX;
        box.y = parent.y + parent.h - h - marginY;
    }
    else if (anchor == UI_BOTTOM_RIGHT)
    {
        box.x = parent.x + parent.w - w - marginX;
        box.y = parent.y + parent.h - h - marginY;
    }
    else
    {
        box.x = parent.x + parent.w / 2 - w / 2 + marginX;
        box.y = parent.y + parent.h / 2 - h / 2 + marginY;
    }

    return box;
}

// 功能：根据窗口锚点和 margin 计算顶层 UI 的屏幕位置。
UIBox makeUIBoxByAnchor(
    int w,
    int h,
    UIAnchor anchor,
    int marginX,
    int marginY
)
{
    return makeUIBoxByParentAnchor(
        makeViewportUIBox(),
        w,
        h,
        anchor,
        marginX,
        marginY
    );
}

// 功能：绘制一个指定颜色的圆角 UI 矩形。
void drawUIBox(UIBox box, COLORREF fillColor, COLORREF borderColor)
{
    setfillcolor(fillColor);
    setlinecolor(borderColor);

    fillroundrect(
        box.x,
        box.y,
        box.x + box.w,
        box.y + box.h,
        30,
        30
    );
}

// 功能：初始化一个默认隐藏的 UI 元素。
UIElement::UIElement()
{
    x = 0;
    y = 0;

    targetX = 0;
    targetY = 0;

    w = 0;
    h = 0;
    targetW = 0;
    targetH = 0;

    anchor = UI_TOP_LEFT;
    marginX = 0;
    marginY = 0;

    parentIndex = -1;

    active = false;
    visible = false;
    interactable = false;

    moveSpeed = 0.2;

    state = UI_HIDDEN;
}

// 功能：立即把当前位置同步到目标位置。
void UIElement::snapToTarget()
{
    x = targetX;
    y = targetY;
}

// 功能：设置当前 UI 元素的父级元素下标，-1 表示父级为窗口。
void UIElement::setParentIndex(int newParentIndex)
{
    parentIndex = newParentIndex;
}

// 功能：获取当前 UI 元素的父级元素下标。
int UIElement::getParentIndex() const
{
    return parentIndex;
}

// 功能：直接设置 UI 元素当前位置。
void UIElement::setPosition(double newX, double newY)
{
    x = newX;
    y = newY;
}

// 功能：直接设置 UI 元素目标位置。
void UIElement::setTargetPosition(double newTargetX, double newTargetY)
{
    targetX = newTargetX;
    targetY = newTargetY;
}

// 功能：获取 UI 元素当前目标 X。
double UIElement::getTargetX() const
{
    return targetX;
}

// 功能：获取 UI 元素当前目标 Y。
double UIElement::getTargetY() const
{
    return targetY;
}

// 功能：按窗口锚点初始化 UI 元素，并默认显示在目标位置。
void UIElement::init(int newW, int newH, UIAnchor newAnchor, int newMarginX, int newMarginY)
{
    w = newW;
    h = newH;
    targetW = newW;
    targetH = newH;

    anchor = newAnchor;
    marginX = newMarginX;
    marginY = newMarginY;

    parentIndex = -1;

    refreshTarget();

    x = targetX;
    y = targetY;

    active = true;
    visible = true;
    interactable = true;

    state = UI_VISIBLE;
}

// 功能：按窗口作为父级重新计算目标位置。
void UIElement::refreshTarget()
{
    refreshTargetByParentBox(makeViewportUIBox());
}

// 功能：切换 UI 锚点，并把新锚点位置作为移动目标。
void UIElement::setAnchor(UIAnchor newAnchor)
{
    anchor = newAnchor;
    refreshTarget();
}

// 功能：根据指定父级 UIBox 重新计算目标位置。
void UIElement::refreshTargetByParentBox(UIBox parentBox)
{
    UIBox box = makeUIBoxByParentAnchor(
        parentBox,
        (int)targetW,
        (int)targetH,
        anchor,
        marginX,
        marginY
    );

    targetX = box.x;
    targetY = box.y;
}

// 功能：立即显示 UI 元素，不播放进入动画。
void UIElement::showInstant()
{
    active = true;
    visible = true;
    interactable = true;

    state = UI_VISIBLE;
}

// 功能：显示 UI 元素，并进入过渡动画状态。
void UIElement::showAnimated()
{
    active = true;
    visible = true;
    interactable = false;

    state = UI_SHOWING;
}

// 功能：兼容旧调用，默认使用动画显示。
void UIElement::show()
{
    showAnimated();
}

// 功能：立即隐藏 UI 元素，不播放退出动画。
void UIElement::hide()
{
    active = false;
    visible = false;
    interactable = false;

    state = UI_HIDDEN;
}

// 功能：进入隐藏动画状态，动画结束后才真正隐藏。
void UIElement::hideAnimated()
{
    active = true;
    visible = true;
    interactable = false;

    state = UI_HIDING;
}

// 功能：平滑推进 UI 元素当前位置，使其靠近目标位置。
void UIElement::update()
{
    if (!active)
    {
        return;
    }

    x = MathUtils::smoothTo(x, targetX, moveSpeed);
    y = MathUtils::smoothTo(y, targetY, moveSpeed);

    w = MathUtils::smoothTo(w, targetW, moveSpeed);
    h = MathUtils::smoothTo(h, targetH, moveSpeed);

    if (fabs(targetX - x) < 0.1)
    {
        x = targetX;
    }

    if (fabs(targetY - y) < 0.1)
    {
        y = targetY;
    }

    if (fabs(targetW - w) < 0.1)
    {
        w = targetW;
    }

    if (fabs(targetH - h) < 0.1)
    {
        h = targetH;
    }

    if (state == UI_SHOWING && x == targetX && y == targetY && w == targetW && h == targetH)
    {
        interactable = true;
        state = UI_VISIBLE;
    }

    if (state == UI_HIDING && x == targetX && y == targetY && w == targetW && h == targetH)
    {
        active = false;
        visible = false;
        interactable = false;
        state = UI_HIDDEN;
    }
}

// 功能：获取 UI 元素当前屏幕矩形。
UIBox UIElement::getBox() const
{
    UIBox box;

    box.x = (int)x;
    box.y = (int)y;
    box.w = w;
    box.h = h;

    return box;
}

// 功能：判断 UI 元素是否参与 update。
bool UIElement::isActive() const
{
    return active;
}

// 功能：判断 UI 元素是否参与 render。
bool UIElement::isVisible() const
{
    return visible;
}

// 功能：判断 UI 元素是否允许交互。
bool UIElement::isInteractable() const
{
    return interactable;
}

// 功能：获取 UI 元素当前生命周期状态。
UIElementState UIElement::getState() const
{
    return state;
}

// 功能：设置目标宽高，并刷新目标定位位置。
void UIElement::setTargetSize(double newTargetW, double newTargetH)
{
    targetW = newTargetW;
    targetH = newTargetH;
    refreshTarget();
}

// 功能：直接设置当前宽高与目标宽高，并刷新目标位置。
void UIElement::setSize(double newW, double newH)
{
    w = newW;
    h = newH;
    targetW = newW;
    targetH = newH;
    refreshTarget();
}

// 功能：获取目标宽度。
double UIElement::getTargetW() const
{
    return targetW;
}

// 功能：获取目标高度。
double UIElement::getTargetH() const
{
    return targetH;
}

// 功能：添加一个 UI 元素，返回它在 UIManager 中的下标。
int UIManager::addElement(UIElement element)
{
    elements.push_back(element);
    return (int)elements.size() - 1;
}

// 功能：判断 UI 元素下标是否有效。
bool UIManager::isValidIndex(int index) const
{
    return index >= 0 && index < (int)elements.size();
}

// 功能：判断 UI 元素在父级链路影响下最终是否可见。
bool UIManager::isElementEffectivelyVisible(int index) const
{
    if (!isValidIndex(index))
    {
        return false;
    }

    const UIElement& element = elements[index];

    if (!element.isVisible())
    {
        return false;
    }

    int parentIndex = element.getParentIndex();

    if (parentIndex < 0)
    {
        return true;
    }

    return isElementEffectivelyVisible(parentIndex);
}

// 功能：获取指定 UI 元素的可写引用。
UIElement& UIManager::getElement(int index)
{
    return elements[index];
}

// 功能：获取指定 UI 元素的只读引用。
const UIElement& UIManager::getElement(int index) const
{
    return elements[index];
}

// 功能：获取指定 UI 元素当前屏幕矩形。
UIBox UIManager::getElementBox(int index) const
{
    if (!isValidIndex(index))
    {
        return makeViewportUIBox();
    }

    return elements[index].getBox();
}

// 功能：根据父级关系刷新指定 UI 元素的目标位置。
void UIManager::refreshElementTarget(int index)
{
    if (!isValidIndex(index))
    {
        return;
    }

    int parentIndex = elements[index].getParentIndex();

    UIBox parentBox;

    if (isValidIndex(parentIndex))
    {
        parentBox = elements[parentIndex].getBox();
    }
    else
    {
        parentBox = makeViewportUIBox();
    }

    elements[index].refreshTargetByParentBox(parentBox);
}

// 功能：按父级关系刷新位置后立即显示指定 UI 元素。
void UIManager::showElementInstant(int index)
{
    if (!isValidIndex(index))
    {
        return;
    }

    refreshElementTarget(index);
    elements[index].showInstant();
    elements[index].snapToTarget();
}

// 功能：按父级关系刷新位置后以动画方式显示指定 UI 元素。
void UIManager::showElementAnimated(int index)
{
    if (!isValidIndex(index))
    {
        return;
    }

    refreshElementTarget(index);
    elements[index].showAnimated();
}

// 功能：按父级关系刷新目标位置后，从指定偏移位置滑入。
void UIManager::showElementAnimatedFromOffset(int index, double offsetX, double offsetY)
{
    if (!isValidIndex(index))
    {
        return;
    }

    refreshElementTarget(index);

    double targetX = elements[index].getTargetX();
    double targetY = elements[index].getTargetY();

    elements[index].setPosition(targetX + offsetX, targetY + offsetY);
    elements[index].showAnimated();
}

// 功能：按父级关系取得正常目标位置，再把目标改为偏移位置并播放隐藏动画。
void UIManager::hideElementAnimatedToOffset(int index, double offsetX, double offsetY)
{
    if (!isValidIndex(index))
    {
        return;
    }

    refreshElementTarget(index);

    double targetX = elements[index].getTargetX();
    double targetY = elements[index].getTargetY();

    elements[index].setTargetPosition(targetX + offsetX, targetY + offsetY);
    elements[index].hideAnimated();
}

// 功能：立即隐藏指定 UI 元素。
void UIManager::hideElementInstant(int index)
{
    if (!isValidIndex(index))
    {
        return;
    }

    elements[index].hide();
}

// 功能：统一更新所有 UI 元素的位置和状态。
void UIManager::update()
{
    for (int i = 0; i < (int)elements.size(); i++)
    {
        int parentIndex = elements[i].getParentIndex();

        UIBox parentBox;

        if (isValidIndex(parentIndex))
        {
            parentBox = elements[parentIndex].getBox();
        }
        else
        {
            parentBox = makeViewportUIBox();
        }

        // UI_HIDING 使用 hideElementAnimatedToOffset() 指定的离场目标，
        // 不能每帧刷新回父级位置，否则隐藏动画会被覆盖。
        if (elements[i].getState() != UI_HIDING)
        {
            elements[i].refreshTargetByParentBox(parentBox);
        }

        elements[i].update();
    }
}
