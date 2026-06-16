#include "Config.h"
#include "GraphicsUtils.h"
#include "Input.h"
#include "Level.h"
// 功能：程序入口，初始化窗口并运行主游戏循环。
int main()
{
    /*
    main 现在只负责程序生命周期：
        1. 初始化窗口
        2. 创建全局输入管理器 InputManager
        3. 创建当前关卡 Level
        4. 主循环中调用 input.update()、level.update()、level.draw()
        5. 退出时释放绘图窗口

    具体关卡内容已经交给 Level 管理。
    */
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(BLACK);

    // 加载全局字体，供 UI 和调试信息使用。
    loadUIFont();



    InputManager input;
    Level level;

    level.init();

    BeginBatchDraw();

    while (true)
    {
        input.update();

        //if (input.isKeyDown(VK_ESCAPE))
        //{
        //    break;
        //}

        level.update(input);

        cleardevice();

        level.draw();

        FlushBatchDraw();

        Sleep(16);
    }

    EndBatchDraw();
    closegraph();

    return 0;
}
