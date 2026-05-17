//#include <graphics.h>
//#include <windows.h>
//#include <cmath>
//#include <iostream>
//#include<conio.h>
//using namespace std;
//
//const double PI = 3.14159265359;
//
//int main()
//{
//    const int viewportWidth = 1600;
//    const int viewportHeight = 900;
//
//    initgraph(viewportWidth, viewportHeight);
//    setbkcolor(BLACK);
//    cleardevice();
//
//
//    IMAGE sourceImage;
//    loadimage(&sourceImage, _T("test2.jpg"));
//
//    int sourceWidth = sourceImage.getwidth();
//    int sourceHeight = sourceImage.getheight();
//
//    if (sourceWidth <= 0 || sourceHeight <= 0)
//    {
//        closegraph();
//        return 0;
//    }
//
//    const int scaleCount = 5;
//    IMAGE scaledImages[scaleCount];
//
//    for (int i = 0; i < scaleCount; i++)
//    {
//        double scale = 1.0 + i / 10.0;
//
//        int scaledWidth = (int)(sourceWidth * scale);
//        int scaledHeight = (int)(sourceHeight * scale);
//
//        loadimage(&scaledImages[i], _T("test2.jpg"), scaledWidth, scaledHeight);
//    }
//
//    double maxScale = 2.0;
//
//    int maxWidth = (int)(sourceWidth * maxScale);
//    int maxHeight = (int)(sourceHeight * maxScale);
//
//    int canvasSize = (int)ceil(sqrt((double)maxWidth * maxWidth + (double)maxHeight * maxHeight));
//
//    if (canvasSize % 2 != 0)
//    {
//        canvasSize++;
//    }
//
//    IMAGE rotateCanvas;
//    IMAGE rotatedCanvas;
//
//    Resize(&rotateCanvas, canvasSize, canvasSize);
//    Resize(&rotatedCanvas, canvasSize, canvasSize);
//
//    int drawX = viewportWidth / 2 - canvasSize / 2;
//    int drawY = viewportHeight / 2 - canvasSize / 2;
//
//    double anchorX = sourceWidth / 2.0;
//    double anchorY = sourceHeight / 2.0;
//
//    double angle = 0;
//
//    int scaleIndex = 0;
//    int scaleStep = 1;
//
//    BeginBatchDraw();
//
//    while (!_kbhit())
//    {
//        double scale = 1.0 + scaleIndex / 10.0;
//
//        IMAGE* currentImage = &scaledImages[scaleIndex];
//
//        SetWorkingImage(&rotateCanvas);
//
//        setbkcolor(BLACK);
//        cleardevice();
//
//        int imageX = (int)(canvasSize / 2.0 - anchorX * scale);
//        int imageY = (int)(canvasSize / 2.0 - anchorY * scale);
//
//        putimage(imageX, imageY, currentImage);
//
//        SetWorkingImage(NULL);
//
//        rotateimage(&rotatedCanvas, &rotateCanvas, angle, BLACK, false, true);
//
//        cleardevice();
//
//        putimage(drawX, drawY, &rotatedCanvas);
//		cout << drawX << " " << drawY << endl;
//        FlushBatchDraw();
//
//        angle += PI / 180.0;
//
//        if (angle >= 2 * PI)
//        {
//            angle = 0;
//        }
//
//        scaleIndex += scaleStep;
//
//        if (scaleIndex <= 0)
//        {
//            scaleIndex = 0;
//            scaleStep = 1;
//        }
//        else if (scaleIndex >= scaleCount - 1)
//        {
//            scaleIndex = scaleCount - 1;
//            scaleStep = -1;
//        }
//
//        Sleep(10);
//    }
//
//    SetWorkingImage(NULL);
//    EndBatchDraw();
//    closegraph();
//
//    return 0;
//}