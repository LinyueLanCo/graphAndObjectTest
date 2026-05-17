//#include <graphics.h>
//#include <conio.h>
//#include <math.h>
//
//int main()
//{
//    // 1. 初始化一个 800x600 的绘图窗口
//    initgraph(800, 600);
//
//    // 2. 加载绝对路径图片
//    IMAGE imgOrig;
//    // 【重要】请将这里的路径替换为你电脑里的绝对路径
//    // 注意 C++ 字符串中反斜杠必须双写 (\\) 进行转义，_T() 宏用于兼容多字节和Unicode字符集
//    loadimage(&imgOrig, _T("C:\\Users\\lpy16\\Downloads\\123.jpg"));
//
//    // 容错处理：如果图片路径不对或加载失败，给出提示并退出
//    if (imgOrig.getwidth() == 0)
//    {
//        outtextxy(10, 10, _T("图片加载失败，请检查绝对路径是否正确！"));
//        _getch();
//        closegraph();
//        return -1;
//    }
//
//    // 动画控制变量
//    double scale = 1.0;       // 当前缩放比例 (起始 1.0 倍)
//    double scaleStep = 0.01;  // 缩放增量步长
//    double angle = 0.0;       // 当前旋转角度 (弧度)
//    double angleStep = -0.03; // 旋转步长 (EasyX 规定：负数代表顺时针旋转)
//
//    IMAGE imgRotated; // 用于保存每帧旋转后的图片对象
//
//    // 开启双缓冲绘图，避免动画过程出现闪烁
//    BeginBatchDraw();
//
//    // 当没有键盘按键按下时，持续循环动画
//    while (!_kbhit())
//    {
//        // --- 逻辑更新阶段 ---
//
//        // 1. 更新缩放比例 (在 0.5 到 1.5 之间来回变化)
//        scale += scaleStep;
//        if (scale >= 1.5 || scale <= 0.5)
//        {
//            scaleStep = -scaleStep; // 触碰边界后反转方向，实现“忽大忽小”
//        }
//
//        // 2. 更新旋转角度 (顺时针叠加)
//        angle += angleStep;
//
//        // --- 图像处理阶段 ---
//
//        // 3. 将原图旋转，存入 imgRotated 中
//        // 参数说明：目标对象, 源对象, 旋转弧度, 填充背景色, 是否自动适应旋转后的尺寸, 是否开启平滑抗锯齿
//        rotateimage(&imgRotated, &imgOrig, angle, BLACK, true, true);
//
//        // --- 计算坐标阶段 ---
//
//        // 获取旋转后图片的实际宽高（由于开启了自动适应，旋转后宽高会不断变化）
//        int rotW = imgRotated.getwidth();
//        int rotH = imgRotated.getheight();
//
//        // 结合缩放比例，计算最终需要绘制在屏幕上的宽高
//        int drawW = (int)(rotW * scale);
//        int drawH = (int)(rotH * scale);
//
//        // 计算居中坐标 (窗口尺寸减去最终绘制尺寸，再除以 2)
//        // 这样不仅实现了你的"水平居中"要求，连"垂直方向"也会完美居中
//        int x = (800 - drawW) / 2;
//        int y = (600 - drawH) / 2;
//
//        // --- 绘制阶段 ---
//
//        // 清除上一帧留下的残影
//        cleardevice();
//
//        // 4. 将旋转后的图像进行缩放并绘制到屏幕
//        // 利用 EasyX 提供的 GetImageHDC() 桥接操作实现动态拉伸，这是 EasyX 下最原生的缩放实现方式
//        StretchBlt(
//            GetImageHDC(NULL),       // 目标 DC：当前窗口的绘图设备上下文
//            x, y, drawW, drawH,      // 目标绘制位置及缩放后的宽高
//            GetImageHDC(&imgRotated),// 源 DC：旋转后图像的设备上下文
//            0, 0, rotW, rotH,        // 源图像的起始位置及原始宽高
//            SRCCOPY                  // 拷贝模式
//        );
//
//        // 刷新缓冲区到屏幕显示
//        FlushBatchDraw();
//
//        // 延时约 15 毫秒，控制动画帧率在大约 60 帧/秒
//        Sleep(15);
//    }
//
//    // 结束双缓冲并关闭窗口
//    EndBatchDraw();
//    closegraph();
//
//    return 0;
//}