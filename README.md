# EasyX C++ 2D 游戏框架练习项目

这是我用 C++ 和 EasyX 做的一个 2D 游戏练习项目。

这个项目现在不急着做成一个完整游戏，更像是我用来理解游戏框架的实验场。我一边加功能，一边拆结构，一边观察数据到底是怎么流动的。现在我最关心的不是“画面有多完整”，而是这些基础系统以后能不能继续扩展。

目前这个 README 主要是写给我自己看的，用来记录：

- 当前项目已经做到哪里。
- 每个模块大约负责什么。
- 游戏主循环启动前发生了什么，以及一帧 tick 里到底发生了什么。
- Camera、Entity、TileMap、动画、碰撞这些系统的数据是如何流动的。
- 后面继续重构时不要把现在已经理清的思路忘掉。

---

## 当前状态

现在项目已经支持这些东西：

- **完全的物理文件拆分**：彻底告别了庞大臃肿的单文件 `game.cpp`，代码被整理到了独立的类文件（`Level`、`Entity`、`CollisionManager` 等）中。
- **Entity 组件直连重构**：剥离了 Entity 身上过时的代理接口，将属性查询和修改直接通过 Getter/Setter 指向内部的 `AnimationPlayer`、`Animator` 以及 `CollisionBox` 组件。
- **背景图层配置化驱动**：完全移除了 `Level` 类中关于天空、云层和树木背景的硬编码，改由外部数据文件 `backgrounds.json` 动态加载和配置。
- **调试逻辑与 Debug UI 完全剥离**：核心关卡逻辑不再插手任何调试工作，UI 渲染、碰撞框显隐按键监听、状态变更日志输出等，均由独立的 `LevelDebugger` 类接管。
- **碰撞管理器状态化升级**：旧有的 `CollisionHandle` 升级为了具备状态管理能力的 `CollisionManager`，它完全接管了实体间的 AABB 重叠检测、去重日志输出以及碰撞反馈分发。
- EasyX 窗口、主循环和双缓冲绘制。
- 世界坐标和 EasyX 屏幕坐标之间的转换。
- 以 `centerX / centerY` 为主锚点的 2D Camera。
- Camera 平滑跟随、鼠标观察偏移、缩放和世界边界限制。
- 视差背景的偏移已经迁移到背景对象的逻辑层 runtime 坐标里，`Renderer` 不再负责计算视差。
- `tileset.png`、`map.txt` 文本地图读取和 tile map 绘制。
- tile 碰撞：根据 tileId 自动生成默认碰撞层，支持完整 solid 和单向平台。
- `sprite` 成为实体、tile、背景这三类单帧绘制对象的统一中间数据，在渲染队列中按 `zIndex` 进行稳定排序。
- 所有被修改的源文件均转为了带 BOM 的 UTF-8 编码，确保多字节字符（中文注释）在 MSVC 中编译正常。

---

## 当前代码排布

项目代码已经完成模块化拆分，各个文件的组织关系如下：

```text
项目根目录
├─ game.cpp                      // 程序入口，负责窗口生命周期与最外层主循环
├─ Level.h / .cpp                // 关卡主舞台，负责调度输入、移动、碰撞与画面绘制的整体流水线
├─ Entity.h / .cpp               // 实体容器类，维护实体的空间状态、朝向，持有各类控制组件
├─ EntityManager.h / .cpp        // 实体管理器，使用静态对象池（200个元素）与活跃/空闲双索引管理实体生命周期
├─ CollisionManager.h / .cpp     // 碰撞与重叠管理器，计算 X/Y 阻挡极限、维护两两重叠状态、清理历史与分发反馈
├─ MovementHandle.h / .cpp       // 运动处理器，根据意图计算重力加速度、跳跃初速度并更新实体的逻辑位置
├─ LevelDebugger.h / .cpp        // 调试管理器，负责 Debug UI 组装、按键响应、物理状态变化日志及绘制
├─ TileMap.h / .cpp              // 瓦片地图类，读取 txt 地图，利用 int** 二维指针管理网格，生成 Tile 实例与碰撞
├─ AnimationPlayer.h / .cpp      // 动画播放器类（原 animatedSprite），推进动画时间轴，计算裁剪框并写入精灵
├─ Animator.h / .cpp             // 动画状态机类，根据实体当前的物理状态决策其应当播放的动画状态
├─ RenderQueue.h / .cpp          // 渲染队列类，收集本帧所有待画的精灵，按 zIndex 进行稳定排序并统一分发
├─ Renderer.h / .cpp             // 渲染绘制类，负责底层的 EasyX 图像绘制与 Debug 边框渲染
├─ Resource.h / .cpp             // 资源管理器类，缓存并管理贴图、音乐、关卡文件等
├─ AnimationClip.h / .cpp        // 动画片段数据定义
├─ AnimationClipManager.h / .cpp // 动画剪辑管理器，加载与缓存所有实体的动画配置 JSON
├─ Camera.h / .cpp               // 2D 摄像机类，管理中心锚点、缩放、视口边界并提供世界与屏幕坐标转换
├─ CameraFollow.h / .cpp         // 相机跟随算法实现，包括平滑跟随与鼠标偏移计算
├─ Controller.h / .cpp           // 控制器定义，包含玩家控制器行为意图的翻译
├─ Sprite.h                      // 精灵单帧绘制数据定义，作为渲染流的统一数据结构
├─ Collision.h                   // 包围盒与碰撞基础定义
├─ Config.h                      // 全局常量与游戏配置参数
├─ GameTypes.h                   // 基础类型定义与全局类型别名
├─ GraphicsUtils.h / .cpp        // 图像处理辅助工具，包含 Alpha 通道绘制等方法
├─ MathUtils.h                   // 数学计算工具
├─ TileTypes.h / .cpp            // 瓦片数据定义
├─ UI.h / .cpp                   // 基础 UI 面板与界面逻辑
└─ README.md                     // 项目说明文档
```

---

## 核心设计思路变化

随着这几次重构的推进，我的思路比以前更加清晰了：

```text
不要为了某个表现效果把特殊规则塞进通用系统；
应该让特殊对象先在自己的逻辑层算出通用数据，
再交给通用系统处理。
```

例如当前的渲染流程：
* `Renderer` 和 `RenderQueue` 完全不在乎谁是谁，它们只接受统一的 `sprite` 数据结构，进行层级排序后调用底层绘制。
* 视差背景由 `BackgroundManager` 独立计算，并在它的逻辑层算出偏移，产生 `sprite` 塞入队列。
* 实体状态和地图瓦片各自负责生成自己的 `sprite` 放入队列。

通过这种“特殊逻辑自我消化，通用接口统一接收”的架构设计，大大降低了各系统之间的耦合，也让 `Level.cpp` 摆脱了以前的大量 `if-else` 分支。

---

## 游戏生命周期流动

为了让我能直观地理解代码运作，下面梳理了整个项目在两个关键阶段的数据流动过程。

### 1. 游戏进入主循环前发生了什么？

在程序刚刚启动，尚未进入 `while(true)` 主循环前，游戏会进行底层的初始化装载：

```text
main() [game.cpp]
  │
  ├── 1. 调用 initgraph() 初始化窗口尺寸，加载全局 UI 字体。
  │
  ├── 2. 实例化全局输入管理器 InputManager。
  │
  ├── 3. 实例化当前关卡控制器 Level level。
  │
  └── 4. 调用 level.init() 开始加载关卡数据：
           │
           ├── 4.1. initResources()：向资源箱中登记关卡所需的全部图片及文本资源物理路径。
           │
           ├── 4.2. loadTemplates()：从 JSON 加载实体属性模板，存入 EntityManager 的模板库。
           │
           ├── 4.3. initMap()：读取 map.txt 文本，生成 TileInstance 列表，并生成默认碰撞网格。
           │
           ├── 4.4. loadEntities()：从 JSON 读取本关初始实体，在连续对象池中初始化分配，并为有动画的实体绑定首帧。
           │
           ├── 4.5. init(uiManager)：初始化调试控制台 Debug 状态与面板信息。
           │
           ├── 4.6. clearHistory()：清空 CollisionManager 中的重叠日志历史去重缓存。
           │
           ├── 4.7. setControlTarget()：默认将键盘控制权限赋给控制列表中处于活跃状态的玩家实体。
           │
           └── 4.8. initBackground()：打开 backgrounds.json，读取并解析出视差背景图层信息，加入背景管理器。
```

---

### 2. 玩家在关卡里进行复合操作时，某一帧 Tick 发生了什么？

这里以一个典型复杂的复合操作场景为例：**“玩家按下向右狂奔并起跳，同时在空中擦过（触碰）了旗杆”**。在这一帧 Tick 里，各个类的数据流动和协作如下：

```text
主循环 Tick 启动
  │
  ├── [Step 1: 输入更新]
  │     InputManager::update() 收集到键盘右方向键、Space 键和 Shift 键被按下。
  │
  ├── [Step 2: 控制意图翻译与物理移动]
  │     Level::updateEntities() 遍历所有存活且活跃的实体：
  │       │
  │       ├── 2.1. PlayerController 将键盘输入翻译为 BehaviorIntent (moveX = 1.0, wantSprint = true, wantJump = true)。
  │       │
  │       └── 2.2. 调用 MovementHandle::update() 传入意图，进行物理计算：
  │              ├── 2.2.1. 检查 wantJump 和 onGround：因为玩家想跳且在地上，瞬间重置 velocityY = JUMP_SPEED，onGround = false，InAir = true。
  │              ├── 2.2.2. 检查 wantSprint：玩家正在移动且在地面，将速度系数翻倍。
  │              ├── 2.2.3. 计算期望水平位移 wantMoveX。
  │              ├── 2.2.4. 调用 collisionManager.getAllowedMoveX()：在活跃实体和固体地图图块中做 AABB 投影求交，发现无阻碍，返回完整位移，更新实体逻辑 x 坐标。
  │              ├── 2.2.5. 累加重力速度：velocityY -= GRAVITY。
  │              ├── 2.2.6. 调用 collisionManager.getAllowedMoveY()：检测在下落/起跳过程中是否撞天花板或踏上单向平台。返回允许的垂直位移，更新实体逻辑 y 坐标。
  │              └── 2.2.7. 调用 collisionManager.limitInWorld()：强制将玩家的逻辑位置钳制在世界长宽像素内。
  │
  ├── [Step 3: 动画状态更新]
  │     实体内持有的 Animator 读取当前的物理移动状态（sprinting = true, jumping = true, InAir = true）：
  │       │
  │       ├── 3.1. 状态机匹配过渡规则，将当前播放的片段从 "run" 切换到 "jumpStart"。
  │       │
  │       └── 3.2. AnimationPlayer（原 animatedSprite）推进动画帧时间，从动画贴图中裁剪出当前的精灵区域并同步写入 renderSprite。
  │
  ├── [Step 4: 相机跟随与视差计算]
  │     Level::updateCamera() 更新摄像机视角：
  │       │
  │       ├── 4.1. CameraFollow 根据玩家最新坐标，利用 Lerp 算法计算出摄像机逻辑中心点。
  │       │
  │       ├── 4.2. Camera 的 limitInWorld() 将其钳制在世界边界内（贴边时生效）。
  │       │
  │       └── 4.3. 调用 backgroundManager.updateRuntimeTransforms(dx, dy) 传入相机参考位移：
  │              各背景图层在自己的逻辑类中根据各自的 parallaxFactor 补偿 runtimeCenterX，实现远景滚得慢、近景滚得快的 3D 纵深视差效果。
  │
  ├── [Step 5: 重叠碰撞检测与反馈分发]
  │     Level::update() 委托 CollisionManager 完成两两重叠状态判定：
  │       │
  │       ├── 5.1. collisionManager.updateOverlapEvents() 扫描活跃实体组合。
  │       │      发现：【Player】的 AABB 包围盒与【Checkpoint (旗杆)】的 AABB 包围盒发生了重叠相交。
  │       │
  │       ├── 5.2. 将双方实体标记为 overlapping = true，并将对方的 ID 写入各自的本帧重叠列表 currentOverlaps。
  │       │
  │       ├── 5.3. 碰撞对去重：组合生成 "Player_Checkpoint" 的唯一键。
  │       │      对比历史缓存 lastOverlapPairs，发现上一帧没有这个重叠，在控制台打印一条“检测到新重叠事件”日志。
  │       │      将本帧所有的重叠键写入 lastOverlapPairs 保存。
  │       │
  │       └── 5.4. collisionManager.resolveEntityOverlaps() 调度实体自治响应：
  │              【Checkpoint 实体】调用其 resolveOverlaps()，发现自身重叠列表中有 PLAYER 类型的实体：
  │                └── 触发升旗动作，播放升旗动画，并将 flagActivatedJustNow 设为 true。
  │
  ├── [Step 6: 关卡级事件与新生实体安全生成]
  │     Level::update() 检查本帧事件：
  │       │
  │       ├── 6.1. 轮询发现 Checkpoint 实体的 flagActivatedJustNow 为 true。
  │       │      重置该标记，调用 entityManager.queueSpawnEntity()，在旗子顶部位置请求生成一个 "Banana" 奖励。
  │       │
  │       └── 6.2. 运行 entityManager.processSpawns()：
  │              在空闲槽位中复活该 Banana，将其索引登记到 activeIndices 中，正式加入世界。
  │
  └── [Step 7: 渲染提交、排序与双缓冲绘制]
        Level::draw() 执行画面渲染：
          │
          ├── 7.1. 清空 RenderQueue 精灵 items。
          │
          ├── 7.2. 依次向队列提交（submit）背景精灵、可见区域内的地图瓦片精灵、以及所有活跃的实体精灵。
          │
          ├── 7.3. 队列 items 按照 zIndex（背景在后，地图在中，角色在前）进行稳定排序。
          │
          ├── 7.4. 渲染器循环调用底层 graphics，将精灵、Debug 碰撞框（玩家和旗子此时为红色）绘制到内存缓冲区。
          │
          └── 7.5. FlushBatchDraw() 统一交换缓冲，将画面瞬间呈现在屏幕上。
```

---

## 运行环境

当前项目主要在以下环境中运行：

- **操作系统**：Windows
- **开发工具**：Visual Studio (支持 MSBuild 编译)
- **图形库**：EasyX (图形模式控制)
- **C++标准**：C++20

用到的核心依赖库与链接项包括：
- `graphics.h` (EasyX 图形接口)
- `windows.h` (Win32 API)
- `fstream` (地图文件解析)
- `Msimg32.lib` (Alpha 通道半透明绘制)
- `winmm.lib` (多媒体定时器及音效播放)

---

## License

代码部分使用 MIT License 授权。

图片、音效、字体等资源如果没有特别说明，就先只当作个人学习测试资源，不一定包含在开源授权范围内。
