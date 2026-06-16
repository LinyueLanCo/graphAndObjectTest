#include "GraphicsUtils.h"

// 功能：注册项目内置字体，供 EasyX 文本绘制使用。
void loadUIFont()
{
    AddFontResourceEx(
        _T("Mojangles.ttf"),
        FR_PRIVATE,
        NULL
    );
}

// 功能：按原图尺寸绘制带 Alpha 通道的图片。
void putimage_alpha(int x, int y, IMAGE* img)
{
    int w = img->getwidth();
    int h = img->getheight();

    BLENDFUNCTION blend;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    AlphaBlend(
        GetImageHDC(NULL),
        x, y, w, h,
        GetImageHDC(img),
        0, 0, w, h,
        blend
    );
}

// 功能：按指定尺寸缩放绘制带 Alpha 通道的图片。
void putimage_alpha(int x, int y, int drawW, int drawH, IMAGE* img)
{
    int sourceW = img->getwidth();
    int sourceH = img->getheight();

    if (drawW < 1)
    {
        drawW = 1;
    }

    if (drawH < 1)
    {
        drawH = 1;
    }

    BLENDFUNCTION blend;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    AlphaBlend(
        GetImageHDC(NULL),
        x, y, drawW, drawH,
        GetImageHDC(img),
        0, 0, sourceW, sourceH,
        blend
    );
}