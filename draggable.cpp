//#include <graphics.h>
//#include <conio.h>
//#include <windows.h>
//#include <tchar.h>
////
////struct Rect
////{
////    int x;
////    int y;
////    int w;
////    int h;
////};
////
////// 让 EasyX 窗口可以自由拉伸
////void enableResizableWindow(HWND hwnd)
////{
////    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
////
////    style |= WS_THICKFRAME;   // 可拖拽边框改变大小
////    style |= WS_MAXIMIZEBOX;  // 显示最大化按钮
////
////    SetWindowLongPtr(hwnd, GWL_STYLE, style);
////
////    SetWindowPos(
////        hwnd,
////        NULL,
////        0, 0, 0, 0,
////        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
////    );
////}
////
////// 同步 EasyX 画布大小和真实窗口客户区大小
////void syncCanvasSize(HWND hwnd, int& lastW, int& lastH)
////{
////    RECT clientRect;
////    GetClientRect(hwnd, &clientRect);
////
////    int w = clientRect.right - clientRect.left;
////    int h = clientRect.bottom - clientRect.top;
////
////    if (w <= 0 || h <= 0)
////    {
////        return;
////    }
////
////    if (w != lastW || h != lastH)
////    {
////        // 关键：EasyX 正确函数是 Resize，不是 resizewindow
////        Resize(NULL, w, h);
////
////        lastW = w;
////        lastH = h;
////    }
////}
////
////void drawCenteredRow(Rect container, int childW, int childH, int count, int gap)
////{
////    int totalW = count * childW + (count - 1) * gap;
////
////    int startX = container.x + (container.w - totalW) / 2;
////    int y = container.y + (container.h - childH) / 2;
////
////    for (int i = 0; i < count; i++)
////    {
////        int x = startX + i * (childW + gap);
////
////        if (i == 0)
////        {
////            setfillcolor(RGB(80, 160, 255));
////        }
////        else
////        {
////            setfillcolor(RGB(255, 160, 80));
////        }
////
////        solidrectangle(x, y, x + childW, y + childH);
////
////        settextcolor(WHITE);
////        setbkmode(TRANSPARENT);
////
////        TCHAR text[32];
////        _stprintf_s(text, _T("Item %d"), i + 1);
////
////        outtextxy(x + 30, y + 40, text);
////    }
////}
////
////int main()
////{
////    initgraph(800, 600);
////
////    HWND hwnd = GetHWnd();
////
////    enableResizableWindow(hwnd);
////
////    int lastW = 800;
////    int lastH = 600;
////
////    BeginBatchDraw();
////
////    while (true)
////    {
////        if (_kbhit())
////        {
////            char ch = _getch();
////
////            if (ch == 27)
////            {
////                break;
////            }
////        }
////
////        syncCanvasSize(hwnd, lastW, lastH);
////
////        int winW = getwidth();
////        int winH = getheight();
////
////        cleardevice();
////
////        Rect windowContainer = { 0, 0, winW, winH };
////
////        int childW = 120;
////        int childH = 100;
////        int count = 2;
////        int gap = 40;
////
////        drawCenteredRow(windowContainer, childW, childH, count, gap);
////
////        settextcolor(WHITE);
////        setbkmode(TRANSPARENT);
////
////        TCHAR info[128];
////        _stprintf_s(info, _T("Window: %d x %d    ESC exit"), winW, winH);
////        outtextxy(20, 20, info);
////
////        FlushBatchDraw();
////        Sleep(16);
////    }
////
////    EndBatchDraw();
////    closegraph();
////
////    return 0;
////}
//
//using namespace std;
//
//int main()
//{
//	initgraph(1600, 900);
//
//	IMAGE *img =new IMAGE;
//
//	Resize(img, 900, 900);
//
//	loadimage(img, _T("assets\\tex\\maps\\background.jpg"));
//
//	putimage(0, 0, img);
//
//	_getch();
//	closegraph();
//	return 0;
//
//
//
//}