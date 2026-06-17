#include "Image2D.h"

Image2D::Image2D()
{
    width = 0;
    height = 0;
}

bool Image2D::load(const TCHAR* path)
{
    loadimage(&image, path);
    width = image.getwidth();
    height = image.getheight();

    if (width == 0 || height == 0)
    {
        return false;
    }

    return true;
}

bool Image2D::load(const TCHAR* path, int loadW, int loadH)
{
    loadimage(&image, path, loadW, loadH);
    width = image.getwidth();
    height = image.getheight();

    if (width == 0 || height == 0)
    {
        return false;
    }

    return true;
}

IMAGE* Image2D::getImage()
{
    return &image;
}

int Image2D::getWidth() const
{
    return width;
}

int Image2D::getHeight() const
{
    return height;
}
