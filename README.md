# EasyX C++ 2D 游戏框架练习项目

这是我用 C++ 和 EasyX 做的一个 2D 游戏练习项目。

这个项目现在不急着做成一个完整游戏，更像是我用来理解游戏框架的实验场。我一边加功能，一边拆结构，一边观察数据到底是怎么流动的。现在我最关心的不是�## 当前状态

现在项目已经支持这些东西：

- EasyX 窗口、主循环和双缓冲绘制。
- 世界坐标和 EasyX 屏幕坐标之间的转换。
- 以 `centerX / centerY` 为主锚点的 2D Camera。
- Camera 平滑跟随、鼠标观察偏移、缩放和世界边界限制。
- 多层背景系统，背景由 `BackgroundLayer / BackgroundManager` 管理，并按 `renderOrder` 决定绘制顺序。
- 背景支持横向平铺、单张世界背景、固定相机背景三种模式。
- 视差背景的偏移已经迁移到背景对象的逻辑层 runtime 坐标里，`Renderer` 不再负责计算视差。
- 背景视差开始引入专用参考锚点 / 视差滚动量，用来避免贴边缩放时被真实 Camera 的边界钳制污染。
- `tileset.png`、`map.txt` 文本地图读取和 tile map 绘制。
- tile 碰撞第一版：根据 tileId 自动生成默认碰撞层，支持完整 solid 和单向平台。
- tile 已经从“二维数组直接绘制”推进到 `TileInstance`，每个 tile 实例拥有中心点、offset、scale 和绘制尺寸。
- 玩家 idle / walk / run / jumpStart / jumpLoop / jumpEnd 动画切换。
- 基础横板移动、重力、跳跃和冲刺。
- 数字键切换当前输入控制实体。
- `F1-F4` 切换相机跟随实体。
- 实体之间的 AABB 阻挡、重叠检测和金币拾取。
- `Renderer` 统一调度背景、地图、实体、UI 和调试框绘制。
- `sprite` 成为实体、tile、背景这类单帧绘制对象的统一中间数据。
- Debug 面板可以显示当前帧真实被 `Renderer` 绘制成功的 background / tile / entity sprite 数量。
- `ResourceManager` 管理玩家动画资源和 `AnimationClip`。
- `Animator` 从实体真实状态里读取数据，并决定当前动画状态。
- `EntityManager` 管理实体列表和双索引缓存，对象池连续存放。
- 实体组件直连重耦：移除了过时代理接口，直接访问组件。
- 视差背景数据化驱动：从 JSON 配置加载背景层参数。
- 调试逻辑独立解耦：提取出 `LevelDebugger` 专职类。
- 碰撞与重叠系统彻底下沉：`CollisionHandle` 升级为 stateful 的 `CollisionManager` 并完全闭包重叠交互逻辑。

目前整个项目已经完成了物理文件拆分，各个核心类都已迁移到了独立的 `.h/.cpp` 文件中。虽然还没有做成 ECS / 完全组件化架构，但逻辑职责已经完成了解耦，大大减轻了以前单文件 `game.cpp` 的臃肿负担。


---

## 当前代码排布

目前项目已经完成了分文件重构。整个工程的文件布局与模块关系排布如下：

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
```

各模块各司其职，底层实现彼此隔离，形成了非常清晰的顶层驱动与底层计算分界。

---

## 最近一次结构整理后的变化

这次 README 距离上一次更新，中间已经有一大段结构变化了。

一开始我只是想让 Debug 面板显示当前画面里绘制了多少元素，但这个需求一路反推出了很多更底层的问题：

```text
想统计真实绘制数量
  -> 必须知道什么东西真的被 Renderer 画出来了
  -> tile 不能只是二维数组循环里的临时绘制
  -> tile 需要变成 TileInstance
  -> entity / tile / background 最好都转换成 sprite
  -> Renderer 只负责统一绘制 sprite
  -> background 也应该从图片数组升级成 BackgroundLayer
  -> 视差不应该写在 Renderer 里，而应该作用在 BackgroundLayer 的逻辑 runtime 坐标上
```

这次变化让我更明确了一个思路：

```text
不要为了某个表现效果把特殊规则塞进通用系统；
应该让特殊对象先在自己的逻辑层算出通用数据，
再交给通用系统处理。
```

比如现在：

```text
Camera:
    只负责基础相机状态、视口范围、缩放和世界到屏幕坐标转换。

BackgroundLayer:
    自己处理视差、fixed、repeat 这类背景规则。

TileMap:
    把地图数据转换成 TileInstance，再转换成 sprite。

Entity:
    把动画状态和当前帧结果写入 sprite。

Renderer:
    不关心这些对象为什么在这里，只负责把 sprite 画出来。
```

这比最早“每个对象自己 draw”或者“Renderer 根据不同类型到处写特殊绘制公式”要清楚很多。

---

## 当前主要模块

### Level

`Level` 现在是关卡调度层。

它负责引导关卡的资源、地图、背景及实体模板加载，也负责每帧按固定流程调度输入、移动更新、重叠检测、相机跟随时机和画面绘制渲染队列的提交。

现在它主要持有：

- `TileMap`
- `Renderer`
- `ResourceManager`
- `CollisionManager` (原 `CollisionHandle`)
- `MovementHandle`
- `EntityManager` (代替了之前的原始 `vector<Entity>` 裸数据遍历)
- `BackgroundManager`
- `UIManager`
- `LevelDebugger` (剥离了庞大的调试面板交互和文本绘制逻辑)
- `RenderQueue` 与 `RenderFrameStats`

我需要记住一点：`Level` 不应该亲自处理太多业务细节。它更像调度者。
移动交给 `MovementHandle`，碰撞及重叠检测交给 `CollisionManager`，调试器交给 `LevelDebugger`，背景层交给 `BackgroundManager`，渲染排序交给 `RenderQueue`，底层绘制交给 `Renderer`，动画切换交给 `Animator`。如果以后 `Level` 又开始变胖，就说明又有逻辑该被拆出去了。

### Renderer

`Renderer` 负责统一绘制。

目前它负责：

- 通用 `sprite` 绘制。
- background sprite。
- tile sprite。
- entity sprite。
- entity debug 碰撞框。
- tile debug 碰撞框。
- background / tile / entity 绘制边界。
- UI。

我之前一直在调整的方向是：所有真正 `putimage` 相关的东西，尽量都收回到 `Renderer` 里。这样外部对象只提供“我要画什么”，而不是自己直接画。

最近这一轮之后，这条线更清楚了：`Renderer` 不应该再负责计算背景视差、tile 位置、entity 动画状态这类业务规则。它应该拿到已经准备好的 `sprite`，然后只做坐标转换、缩放绘制、调试边界和真实绘制数量统计。

这是目前比较清楚的一条边界：

```text
Entity / TileMap / BackgroundLayer / UI:
    在自己的逻辑层准备绘制所需的数据。

Renderer:
    把这些数据转换为 EasyX 绘制调用。
```

现在 Debug 面板里的 Render 数据也应该按这个思路理解：它不是“场景里存在多少对象”，而是“本帧真实有多少个 sprite 被 Renderer 画出来了”。

### RenderQueue

`RenderQueue` 渲染队列是我为了实现“所有东西统一绘制且严格控制层级”而做出来的关键重构。

以前每个对象（背景、瓦片、实体）都是自己 draw，谁先调 draw 谁就在底下，完全依赖代码书写顺序。一旦我想做更复杂的 Z-Sorting（层级排序）或者按 zIndex 决定谁遮挡谁，就得把整个代码结构翻过来。

现在的做法是：
- **解耦的精灵收集**：在 `Level::draw()` 开始时，清空 `RenderQueue`，接着向它提交（submit）这一帧所有需要画出来的精灵（`Sprite`）。
- **稳定排序（Z-Sorting）**：所有精灵提交完毕后，`RenderQueue` 使用 `std::stable_sort` 按照精灵的 `zIndex`（背景最远、瓦片在中、角色在前、UI和调试框在最前）进行稳定层级排序。
- **批量绘制分发**：排序完成后，将排好序的精灵队列提交给 `Renderer` 进行顺序的底层 EasyX 渲染绘制。

这套设计极大地规范了渲染流。而且配合 `RenderFrameStats`，我们能准确获知这一帧真正被渲染器绘制到屏幕上的背景层、瓦片和实体的精灵总数，对于做视口剔除和性能监控非常有帮助。

### Entity

`Entity` 现在已经比最早精简了很多，但它还不是最终形态。

它目前仍然保存：

- 位置。
- 速度。
- 朝向。
- 存活、落地、跳跃、god 模式等状态。
- `CollisionBox`。
- `Animator`。
- `AnimationPlayer`（原 `animatedSprite` 命名已被纠正为动画播放器）。
- `Sprite`（渲染单帧中间件数据结构）。

已经拆出去的部分：

- 碰撞盒计算：`CollisionBox`。
- 动画状态选择：`Animator`。
- 动画资源加载：`ResourceManager`。
- 渲染调度：`Renderer`。
- 碰撞检测与阻挡位移限制：`CollisionManager`。
- 移动、重力、跳跃更新：`MovementHandle`。

最近我将 `Entity` 进行了组件直连重构，彻底剥离了其身上的转发型代理接口。现在外部可以直接通过 `getAnimation()`、`getAnimator()` 和 `getCollisionBox()` 接口拿到对应的组件引用进行状态操作，避免了 `Entity` 类本身的接口膨胀。
但是现在的 `Entity` 仍然不是严格的组件化结构。它只是“把很多逻辑拆成了对象”，还没有做到“每个实体按需挂载不同组件”。

后面如果继续做 prefab / component / ECS 一类的结构，要继续把这两件事分开：

```text
对象身份：
    这个东西是谁，有什么 id，在哪里，是否存在。

对象能力：
    它能不能移动，能不能碰撞，能不能播放动画，能不能触发事件。
```

### EntityManager

`EntityManager` 是我的实体总管家。以前实体是直接放在 `Level` 的裸 `vector<Entity>` 里，每帧更新都得全局遍历，且实体的动态生成和销毁很容易在迭代过程中导致迭代器失效，极其危险。

为此，我设计了 `EntityManager`，它的职责包括：

- **预分配连续对象池**：内部使用 `std::vector<Entity>` 固定存放 200 个实体实例，避免在游戏运行时频繁进行内存分配与释放。
- **双索引列表设计**：维护 `activeIndices`（活跃实体的槽位索引）和 `deadIndices`（空闲可复用的槽位索引）两个列表。遍历和更新物理/动画时只遍历活跃索引，销毁时只需将其索引从 active 转移到 dead，最大化避免无用遍历。
- **帧末安全延迟生成**：通过 `queueSpawnEntity()` 将本帧内因为各种碰撞或事件产生的新实体（如吃金币时生成的漂浮文字、旗杆触发的香蕉奖励）暂时登记在 `spawnQueue` 队列中，等一帧逻辑完全更新完毕后在帧末的 `processSpawns()` 里安全实例化并分配槽位加入活跃列表，完美规避了迭代中修改容器的崩溃风险。
- **自增 ID 导航**：维护 `nextEntityId` 唯一计数器，并使用 `idToIndex`（哈希表）实现通过实体 ID 常数时间寻址实体的读写指针。
- **实体模板载入**：统一读取 `entity_templates.json` 实体模板配置，解析为 `EntityTemplate` 并存入模板库，实现实体属性与配置的外置数据驱动。

### Animator

`Animator` 现在负责动画状态切换。

它读取实体当前的真实状态，比如：

- 水平速度。
- 是否在空中。
- 是否落地。
- 是否正在起跳。
- 是否正在冲刺。
- 朝向。

然后它决定当前应该播放哪个动画。

这里我要继续保持一个原则：

```text
Animator 可以读取游戏状态，
但不要制造会影响玩法逻辑的状态。
```

比如速度、朝向、是否落地、是否撞到头顶，这些都不应该由 `Animator` 决定。`Animator` 只是根据这些状态选择表现。

如果以后有一些动画内部状态，比如 `wasInAir`，只要它只服务于动画切换，不反过来控制实体逻辑，就可以留在 `Animator` 内部。

### AnimationClip / AnimationPlayer / Sprite

目前这三层比之前更清楚了一些。

我希望最后大概是这样：

```text
AnimationClip:
    定义一段动画的数据。
    比如图片资源、帧宽高、帧数量、间隔、播放速度、是否循环。

AnimationPlayer:
    负责推进时间，算出当前应该播放第几帧，以及这一帧在 sprite sheet 上的裁剪区域。

Sprite:
    保存最终当前帧的绘制信息。
    比如使用哪张图、裁剪坐标、绘制偏移、逻辑绘制尺寸、世界绘制中心点和世界绘制宽高。

Renderer:
    拿 Sprite 的结果，最终调用 EasyX 绘制。
```

最近的一个重要变化是：`sprite` 不再只是 entity 动画的结果，它正在变成所有单帧绘制对象的统一中间层。

现在大致是：

```text
Entity:
    Animator / AnimationPlayer 算出当前帧
    -> 写入 sprite
    -> Renderer::drawSprite()

TileInstance:
    TileMap 根据 tile 实例数据生成 sprite
    -> Renderer::drawSprite()

BackgroundLayer:
    BackgroundManager 更新 runtime 逻辑位置
    -> BackgroundLayer 生成 sprite
    -> Renderer::drawSprite()
```

这个变化很重要。它让我后面做视口裁剪、绘制统计、绘制边界调试时，不需要分别给 entity / tile / background 写完全不同的逻辑。

我已经将原本的 `animatedSprite` 彻底更名为 `AnimationPlayer`，明确了其作为“动画播放器”的独立逻辑职责，避免了和底层 Z-Sorting 渲染用到的 `Sprite` 精灵概念混淆。

### BackgroundLayer / BackgroundManager

背景现在已经不再是 `Level` 里几张图片和几个绘制参数了。

当前背景相关结构大概是：

```text
BackgroundLayer:
    保存单个背景层的数据。
    包括图片、renderOrder、parallaxFactor、drawMode、基础中心点和 runtime 中心点。

BackgroundManager:
    保存所有背景层。
    负责排序，并在 update 阶段更新每个背景层本帧的 runtime transform。

Renderer:
    只负责把 BackgroundLayer 生成出来的 sprite 画出来。
```

现在支持三种背景模式：

```cpp
BACKGROUND_REPEAT_X
BACKGROUND_SINGLE_WORLD
BACKGROUND_FIXED_CAMERA
```

我现在对背景的理解是：它虽然不是可交互实体，但它可以被看成一种有特殊移动规则的场景对象。

所以背景视差不应该继续塞进 `Camera` 或 `Renderer`。更合理的数据流是：

```text
Camera / Level:
    算出用于背景视差的参考位移。

BackgroundManager:
    把这个位移作用到每个 BackgroundLayer 的 runtimeCenterX / runtimeCenterY。

BackgroundLayer:
    根据 runtime 坐标生成 sprite。

Renderer:
    统一绘制 sprite。
```

这就是最近这轮重构里最重要的思路变化：

```text
特殊规则先作用到对象自己的逻辑层；
通用系统只处理通用数据。
```

这里的 `centerX / centerY` 和 `runtimeCenterX / runtimeCenterY` 也要分清楚：

```text
centerX / centerY:
    设计时的基础位置。
    我摆放背景对象时真正想保存的坐标。

runtimeCenterX / runtimeCenterY:
    当前帧实际用于绘制的位置。
    可以被视差、fixed 等规则修改。
```

后面如果要动态生成背景对象来实现平铺，这套结构可以继续沿用。那时每一个平铺出来的背景也应该是一个拥有自己 sprite 的对象，而不是 Renderer 里的临时绘制特例。

#### 背景层不一定都属于同一种空间

现在背景层虽然统一通过 `sprite` 进入 `Renderer`，但它们在设计含义上不一定都属于同一种空间。

大致可以先这样区分：

```text
屏幕空间背景：
    天空、纯色远景、最远云层。
    这类背景几乎不应该被世界坐标影响，也不应该强烈响应 Camera zoom。

视差空间背景：
    远山、云层、远处建筑。
    这类背景会根据 Camera 横向移动产生不同速度的视差，
    但它们不一定应该完全等同普通世界物体。

世界空间背景：
    近景岩壁、贴近 tile 的建筑、地面附近的大型装饰。
    这类背景可以接近普通 tile 的移动速度，甚至完整响应世界 zoom。
```

所以现在的 `BACKGROUND_REPEAT_X / BACKGROUND_SINGLE_WORLD / BACKGROUND_FIXED_CAMERA` 更像是“绘制模式”，而不是严格的“空间类型”。

后面如果继续扩展，可以考虑再加一层空间语义：

```cpp
enum BackgroundSpace
{
    BACKGROUND_SCREEN_SPACE,
    BACKGROUND_PARALLAX_SPACE,
    BACKGROUND_WORLD_SPACE
};
```

这样以后就能更清楚地区分：

```text
这个背景层应该怎么铺？
这个背景层应该吃多少 Camera 位移？
这个背景层应该吃多少 Camera zoom？
这个背景层是否应该被世界边界影响？
```

当前的 `zoomFactor` 也可以按这个方向继续使用。它目前更像预留字段，还没有完全形成独立的背景缩放规则。

后面可以用它描述背景层对 Camera zoom 的响应程度：

```text
zoomFactor = 0.0:
    不响应世界 zoom，适合天空 / 最远背景。

zoomFactor = 0.5:
    部分响应 zoom，适合中远景。

zoomFactor = 1.0:
    完全响应世界 zoom，适合近景 / 贴近 tile 的背景。
```

这样可以避免所有背景层都像普通世界 sprite 一样完整吃 `gCamera.zoom`。

#### 视差效果的核心逻辑

现在背景视差不是直接在屏幕坐标上改 `drawX`，也不是让 `Camera` 专门为背景做一套特殊转换，而是在背景对象自己的逻辑层修改 runtime 坐标。

当前横向视差的大致公式是：

```cpp
runtimeCenterX = centerX + parallaxOffsetX * (1.0 - parallaxFactor);
```

这里几个量的含义是：

```text
centerX:
    背景层设计时的基础世界中心点。

parallaxOffsetX:
    背景视差参考相机相对初始位置移动了多少。

runtimeCenterX:
    当前帧背景层真正用于生成 sprite 的世界中心点。

parallaxFactor:
    背景在屏幕上相对普通地图物体的横向移动速度比例。
```

因为最终绘制仍然会经过 Camera：

```text
screenX = (worldX - cameraCenterX) * zoom
```

所以当玩家向右走、Camera 也向右移动时，如果背景 `worldX` 完全不动，背景会像普通地图一样向屏幕左侧移动。

为了让远景移动得更慢，背景对象的逻辑坐标需要向 Camera 移动方向补偿一部分：

```text
玩家 / Camera 向右移动 dx
  -> 普通地图在屏幕上向左移动 dx
  -> 背景 runtimeCenterX 向右补偿一部分
  -> 背景在屏幕上只向左移动 dx * parallaxFactor
```

所以现在 `parallaxFactor` 可以这样理解：

```text
parallaxFactor = 0.0:
    背景几乎固定在屏幕上，适合最远天空。

parallaxFactor = 0.5:
    背景以地图一半的速度移动。

parallaxFactor = 1.0:
    背景像普通世界物体一样移动，没有视差差异。

parallaxFactor > 1.0:
    背景会比地图移动得更快，一般不适合作为远景背景。
```

我之前一开始把参数设得太小，比如 `0.04 / 0.12 / 0.25 / 0.4`，结果背景几乎跟着 Camera 一起补偿，看起来像是跟着角色往右跑。后来把参数整体调大以后，效果才更接近正常视差。

现在我需要记住：在这套“背景也是世界空间 sprite”的结构下，背景逻辑坐标跟着 Camera 方向移动一部分不是 bug，而是为了抵消一部分 Camera 位移，让背景在屏幕上移动得更慢。

#### layer4 贴边缩放时的坐标矫正实例

这里用当前代码里的第 4 层背景举例。

这一层初始化时大致是：

```cpp
BackgroundLayer layer4;
layer4.load(
    _T("assets\\tex\\maps\\Clouds 5\\5.png"),
    4,
    0.88,
    0.85,
    true,
    BACKGROUND_REPEAT_X
);
layer4.setDrawData(800, 450, WINDOW_WIDTH, WINDOW_HEIGHT);
```

也就是说：

```text
layer4.centerX = 800
layer4.centerY = 450
layer4.drawW = 1600
layer4.drawH = 900
layer4.parallaxFactor = 0.88
```

因为 `sprite` 使用中心点和绘制尺寸，所以这一层在没有视差修正时的世界绘制矩形是：

```text
worldLeft   = runtimeCenterX - drawW / 2 = 800 - 800 = 0
worldRight  = runtimeCenterX + drawW / 2 = 800 + 800 = 1600
worldBottom = runtimeCenterY - drawH / 2 = 450 - 450 = 0
worldTop    = runtimeCenterY + drawH / 2 = 450 + 450 = 900
```

这里要注意：

```text
背景层的“绘制坐标原点”不是 centerX / centerY。
centerX / centerY 是世界中心点。
真正交给 EasyX 绘制时使用的是 worldLeft / worldTop 转换后的屏幕坐标。
```

也就是：

```cpp
drawX = gCamera.worldToScreenX(worldLeft);
drawY = gCamera.worldToScreenY(worldTop);
```

##### 出生点贴左边时，zoom = 1

玩家出生点靠近地图左侧时，Camera 虽然想跟随玩家，但会被 `limitInWorld()` 钳制。

当 `zoom = 1` 时：

```text
visibleW = WINDOW_WIDTH / zoom = 1600 / 1 = 1600
halfW = 800
```

如果玩家 X 小于 800，Camera 的真实中心点会被钳制到：

```text
gCamera.centerX = 800
viewLeft = centerX - halfW = 800 - 800 = 0
```

这时 `layer4` 如果没有额外视差偏移：

```text
runtimeCenterX = 800
worldLeft = 800 - 800 = 0
```

代入屏幕转换公式：

```text
screenX = WINDOW_WIDTH / 2 + (worldLeft - gCamera.centerX) * zoom
        = 800 + (0 - 800) * 1
        = 0
```

所以在出生点贴左边、`zoom = 1` 时，`layer4` 这一张背景的左上角屏幕 X 正好是 0。

这和 tile 的情况类似：tile 地图从世界 X = 0 开始，Camera 贴左边时 `viewLeft` 也是 0，所以视觉上不会露出世界外。

##### 出生点贴左边时，从 zoom = 1 放大到 zoom = 2

当 `zoom = 2` 时：

```text
visibleW = WINDOW_WIDTH / zoom = 1600 / 2 = 800
halfW = 400
```

如果玩家仍然在左侧出生点附近，Camera 真实中心点会被钳制到：

```text
gCamera.centerX = 400
viewLeft = centerX - halfW = 400 - 400 = 0
```

这一步很关键：

```text
玩家没有真的向左移动；
只是 zoom 改变后，逻辑视口宽度变小；
limitInWorld 为了让 viewLeft 继续等于 0，
把真实 Camera centerX 从 800 修正到了 400。
```

对于 tile 来说，这没有问题。因为 tile 从 `worldX = 0` 开始绘制：

```text
screenX = 800 + (0 - 400) * 2 = 0
```

tile 左边界仍然贴着屏幕左边。

但背景层如果把这次 `centerX: 800 -> 400` 误认为是“Camera 真的横向移动了 -400”，就会出现额外偏移。

按照当前视差公式：

```cpp
runtimeCenterX = centerX + parallaxOffsetX * (1.0 - parallaxFactor);
```

如果直接用真实 Camera center 计算：

```text
parallaxOffsetX = gCamera.centerX - parallaxOriginX
                = 400 - 800
                = -400
```

而 `layer4.parallaxFactor = 0.88`，所以：

```text
runtimeCenterX = 800 + (-400) * (1.0 - 0.88)
               = 800 - 48
               = 752

worldLeft = runtimeCenterX - drawW / 2
          = 752 - 800
          = -48
```

再交给真实 Camera 绘制：

```text
screenX = 800 + (worldLeft - gCamera.centerX) * zoom
        = 800 + (-48 - 400) * 2
        = -96
```

也就是说，背景左边界会从屏幕 X = 0 额外滑到 X = -96。

这个偏移不是玩家移动造成的，而是：

```text
zoom 改变
  -> limitInWorld 修正真实 Camera center
  -> 背景视差误把这个修正当成 Camera 平移
  -> runtimeCenterX 被错误补偿
  -> 最终绘制坐标产生额外滑动
```

##### 使用视差参考锚点后的结果

如果使用 `parallaxCameraX / parallaxReferenceX` 这类背景专用参考锚点，并且这个参考锚点不把“贴边缩放导致的 center 修正”算作真实横向移动，那么在玩家没有横向移动时：

```text
parallaxOffsetX = 0
```

于是：

```text
runtimeCenterX = 800 + 0 * (1.0 - 0.88)
               = 800

worldLeft = 800 - 800
          = 0
```

最终绘制：

```text
screenX = 800 + (0 - 400) * 2
        = 0
```

这才是我想要的效果：

```text
zoom 放大后，真实 Camera centerX 可以为了贴边从 800 变成 400；
但是背景视差层不把这次变化当成玩家平移；
layer4 的世界绘制左边界仍然保持在 worldX = 0；
最后通过真实 Camera 转换后，屏幕绘制左上角仍然是 X = 0。
```

也就是说，`parallaxCameraX / parallaxReferenceX` 的作用不是替代真实 Camera 绘制，而是决定：

```text
哪些 Camera center 变化应该影响背景 runtimeCenterX；
哪些变化只是 zoom + limitInWorld 带来的技术修正，不应该影响背景 runtimeCenterX。
```

##### 这个例子里的关键结论

```text
真实 gCamera.centerX：
    负责最终 worldToScreen，必须被 limitInWorld 钳制。

parallaxReferenceX：
    负责背景层的视差参考，不一定等于真实 gCamera.centerX。

layer4.runtimeCenterX：
    由 parallaxOffsetX 和 parallaxFactor 算出，是这一帧背景参与绘制的世界中心点。

worldLeft / worldTop：
    才是最终转换成 EasyX drawX / drawY 的世界绘制原点。
```

这个例子也说明：

```text
背景坐标矫正不是为了让背景完全不动，
而是为了过滤掉“不应该被视差系统响应的 Camera 修正量”。
```

#### 横向平铺的处理

`BACKGROUND_REPEAT_X` 现在仍然是在绘制阶段围绕背景层的 `runtimeCenterX` 横向生成多个 sprite，保证当前视口能被覆盖。

也就是说当前结构是：

```text
一个 BackgroundLayer
  -> update 阶段算出一个 runtimeCenterX
  -> draw 阶段围绕这个 runtimeCenterX 生成多个横向 sprite
```

这还不是真正的“动态生成多个背景对象”。后面如果要继续统一对象模型，可以把平铺出来的每一张背景都变成独立 background object。到那时，平铺逻辑就可以从“生成多个 sprite”升级成“维护多个背景对象实例”。

### TileMap

`TileMap` 现在负责三类数据：

- 视觉层：地图上每个格子画哪个 tile。
- 碰撞层：地图上每个格子对应什么碰撞规则。
- tile 实例层：把二维数组里的 tile 转换成可单独获取、可带 offset/scale 的 `TileInstance`。

现在 tile 不再只是“绘制二维数组时临时算出来的一个格子”。它已经有了更明确的数据对象：

```text
TileInstance:
    tileId
    row / col
    centerX / centerY
    offsetX / offsetY
    scaleX / scaleY
    drawW / drawH
```

这样做的目的不是马上把 tile 做成完整 GameObject，而是先让我能具体拿到某一个 tile，并且能对单个 tile 做基础变换和调试显示。

现在 tile 绘制流程大概是：

```text
map.txt / 二维数组
  -> 生成 TileInstance 列表
  -> TileMap::buildSpriteFromTileInstance()
  -> Renderer::drawSprite()
```

这也让 tile 开始和 entity / background 走同一条 sprite 绘制路线。

目前碰撞规则先做得比较保守：

```cpp
TILE_COLLISION_NONE
TILE_COLLISION_SOLID
TILE_COLLISION_TOP_HALF_ONE_WAY
TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY
TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY
```

现在还没有做非常精细的任意形状碰撞。当前思路是先定义几种常用预设，然后要求参与碰撞的 tile 素材尽量遵守这些规则。

这个选择目前比较合理，因为我的目标不是做一个通用物理引擎，而是先让横板地图真的能玩。

### CollisionManager

`CollisionManager`（原 `CollisionHandle`）负责碰撞检测、位移修正以及活跃实体之间的重叠事件分发。

现在它主要处理：

- 实体和实体的 AABB 阻挡。
- 实体和瓦片地图 tile 的 solid 阻挡。
- 单向平台的站立与下坠物理检测。
- 世界边界限制。
- 状态化实体间的重叠检测与去重事件分发。

这里的关键是：物理碰撞不只是“有没有碰到”，更核心的是“碰到之后如何限制并修正玩家/怪物的最终位移”。因此当 `MovementHandle` 驱动实体移动时，会先向 `CollisionManager` 调用 `getAllowedMoveX` / `getAllowedMoveY` 查询本帧物理修正后允许移动的安全距离。

另外，我为 `CollisionManager` 增加了**状态化重叠事件管理**：
- 它在内部维护了一个上一帧发生重叠的实体配对记录（`lastOverlapPairs`），以防止同一种重叠触发每帧重复打印日志或重复处理。
- `updateOverlapEvents()` 每帧在活跃实体间做两两 overlap 包包围盒相交扫描，并把重叠结果填充到实体的 `currentOverlaps` 列表中。
- `resolveEntityOverlaps()` 统一调度活跃的实体，让它们各自执行 `resolveOverlaps()` 做出对应的自治反应（如角色收集金币、触碰旗杆等）。

### MovementHandle

`MovementHandle` 负责移动相关逻辑：

- 水平移动。
- 冲刺。
- 跳跃。
- 重力。
- god 模式移动。

它现在还是作为 `Level` 里的一个系统对象存在。后面如果真做组件化，这部分可能会变成不同 entity 可挂载的 movement component，或者变成一个 system 去处理所有带 movement 数据的对象。

我现在先不急着彻底重构它。只要记住这个方向就行：

```text
如果 MovementHandle 只服务于某个实体类型：
    它更像组件。

如果 MovementHandle 负责统一处理所有带移动数据的对象：
    它更像系统。
```

### LevelDebugger

`LevelDebugger` 是从原本臃肿的 `Level` 逻辑中剥离出来的调试管理器。它的存在是为了让游戏的主要执行流程与各种调试界面的显示和交互完全解耦。

它目前负责：
- **Debug UI 组装**：将帧率、物体坐标、当前控制实体、摄像机位置等调试数据实时收集并包装成 `DebugPanelData` 递交给 UI 组件更新。
- **键盘调试开关监听**：在 `handleInput()` 里响应 `F5-F11` 等调试控制按键，包括开关碰撞框绘制、开关瓦片边界绘制、临时隐藏/显示特定的 Debug 面板分区。
- **物理状态日志**：每帧监控实体物理状态的变化（如 `onGround`、`blockedByWorld`），当状态突变时向控制台打印带时间戳的清晰日志，方便我追溯跳跃或撞墙的瞬间。
- **辅助线绘制协调**：协调并调度 `Renderer` 在场景中画出实体的包围盒、地图阻挡线、以及各类可绘制对象的尺寸边界。

---

## Camera 与世界/屏幕坐标转换

Camera 现在已经是项目里最重要的数据流之一了。它的复杂度其实不比 Entity 那条线低，因为这里同时牵涉到：

- 世界坐标。
- 屏幕坐标。
- EasyX 的 Y 轴方向。
- zoom。
- 视口范围。
- 世界边界。
- 鼠标偏移。
- 视差背景。

这一节主要是写给以后忘了相机逻辑的自己看的。

### 两套坐标

当前工程里同时存在两套坐标：

```text
世界坐标：
    逻辑坐标。
    原点在左下角。
    Y 轴向上。

EasyX 屏幕坐标：
    绘制坐标。
    原点在窗口左上角。
    Y 轴向下。
```

Entity 的 `x / y` 表示实体中心点。Camera 不改变 Entity 的逻辑坐标，它只负责在渲染时提供转换规则。

换句话说：

```text
Entity 不知道自己最后画在屏幕哪里。
Renderer 通过 Camera 把 Entity 的世界坐标换算成屏幕坐标。
```

### 旧锚点：视口左下角

早期 Camera 主要保存：

```cpp
double x;
double y;
```

这里的 `x / y` 表示视口左下角在世界坐标中的位置。

旧的转换公式大概是：

```cpp
screenX = (worldX - cameraX) * zoom;
screenY = WINDOW_HEIGHT - (worldY - cameraY) * zoom;
```

这个模型在不缩放时很好理解。`cameraX / cameraY` 就像世界被往反方向挪了一下。

但问题出在平滑 zoom。

只要 `zoom` 变化，逻辑可见范围就会变化：

```cpp
visibleW = WINDOW_WIDTH / zoom;
visibleH = WINDOW_HEIGHT / zoom;
```

如果 Camera 的主状态还是左下角，那么“正确的左下角位置”会跟着 zoom 一起变。与此同时，左下角本身又在平滑跟随目标，于是画面容易出现一种额外漂移。

旧问题可以这样理解：

```text
zoom 改变
  -> visibleW / visibleH 改变
  -> 正确的 camera.x / camera.y 改变
  -> camera.x / camera.y 又通过 lerp 慢慢追
  -> 缩放时画面看起来被多拖了一下
```

这就是我后来要把 Camera 改成中心点锚点的原因。

### 新锚点：视口中心点

现在 Camera 的主状态是：

```cpp
double centerX;
double centerY;
double zoom;

double targetCenterX;
double targetCenterY;
double targetZoom;
```

`centerX / centerY` 是相机真正跟随和平滑的点。视口上下左右不再作为主状态保存，而是每次根据中心点和 zoom 推出来。

当前公式是：

```cpp
visibleW = WINDOW_WIDTH / zoom;
visibleH = WINDOW_HEIGHT / zoom;

viewLeft = centerX - visibleW / 2.0;
viewRight = centerX + visibleW / 2.0;
viewBottom = centerY - visibleH / 2.0;
viewTop = centerY + visibleH / 2.0;
```

世界坐标转屏幕坐标的核心公式：

```cpp
screenX = WINDOW_WIDTH / 2.0 + (worldX - centerX) * zoom;
screenY = WINDOW_HEIGHT / 2.0 - (worldY - centerY) * zoom;
screenSize = worldSize * zoom;
```

这条公式的含义很重要：

```text
先求世界点相对 Camera 中心点的偏移，
再乘以 zoom，
最后把结果放到屏幕中心。
```

这样 Camera 的主锚点就稳定了。

```text
zoom 只改变“看多大”和“放多大”；
centerX / centerY 仍然表示我正在看哪里。
```

### Camera 每帧更新流程

当前一帧里 Camera 大概这样更新：

```text
handleCameraInput()
  -> 根据 B / V 设置 targetZoom

updateCamera()
  -> updateCameraFollow()
      -> gCamera.updateZoom()
      -> 根据鼠标屏幕偏移计算世界偏移
      -> gCamera.followSmooth(targetX, targetY, worldWidth, worldHeight, offsetX, offsetY)
          -> targetCenter = 跟随目标位置 + 鼠标世界偏移
          -> center 通过 lerp 靠近 targetCenter
          -> limitInWorld() 限制 center，不让视口看到世界外

updateParallaxReference() / updateParallaxCamera()
  -> 更新背景视差参考锚点
  -> 这个锚点不一定等于最终被 limitInWorld 钳制后的真实 gCamera.centerX
  -> 它用于过滤贴边缩放导致的 center 修正
```

这里有一个顺序要记住：

```text
先 updateZoom()
再 followSmooth()
```

因为 `followSmooth()` 里的世界边界限制，需要用最新的 zoom 推导当前视口大小。

### 世界边界限制

Camera 现在限制的是中心点，不是旧的左下角。

```cpp
halfW = visibleW / 2.0;
halfH = visibleH / 2.0;

centerX = clamp(centerX, halfW, worldWidth - halfW);
centerY = clamp(centerY, halfH, worldHeight - halfH);
```

如果世界尺寸比当前视口还小，那就让相机中心落在世界中心：

```cpp
centerX = worldWidth / 2.0;
centerY = worldHeight / 2.0;
```

这里要记住：

```text
Camera 跟随玩家，不代表玩家永远在屏幕中心。
```

如果玩家贴边，Camera 会优先保证视口不露出世界外。

### Camera 边界限制与关卡设计边界

`limitInWorld()` 的职责是防止视口看到世界外区域。它应该是技术兜底，而不应该承担所有镜头设计职责。

如果一个关卡经常让 Camera 直接贴到世界边界，然后又频繁缩放，就很容易让这些东西纠缠在一起：

```text
zoom
visibleW / visibleH
limitInWorld()
真实 Camera center
背景视差参考位移
```

更稳的设计是把“玩家能去哪里”和“Camera 怎么构图”分开：

```text
玩家边界：
    由碰撞体、墙、空气墙、门、悬崖等关卡结构限制玩家移动。

Camera 边界：
    尽量通过 CameraZone / CameraAnchor / POI 控制镜头构图。

limitInWorld：
    只作为最后保险，防止极端情况下露出世界外。
```

也就是说，玩家不一定要真的走到世界坐标边界。很多情况下，应该让玩家被场景碰撞体挡住，而不是让 Camera 频繁贴到整个世界的最外侧。

这种思路不是逃避技术问题，而是把相机系统从“被动防越界”提升到“主动做构图”。

### CameraMode / CameraAnchor / CameraZone

后面 Camera 不应该只有“跟随玩家”一种模式。

可以先考虑增加几种模式：

```cpp
enum CameraMode
{
    CAMERA_FOLLOW_TARGET,   // 跟随玩家或指定实体
    CAMERA_FIXED_POINT,     // 固定到一个点或 CameraAnchor
    CAMERA_LOCK_REGION,     // 锁定在某个房间 / 区域
    CAMERA_SCRIPTED         // 剧情 / 演出接管
};
```

其中 `CameraAnchor` 可以是一个没有渲染信息的逻辑对象：

```cpp
struct CameraAnchor
{
    double x;
    double y;
    double targetZoom;
};
```

玩家进入某个 POI / 房间 / Boss 区域时，Camera 可以切换到 `CAMERA_FIXED_POINT` 或 `CAMERA_LOCK_REGION`，不再直接跟随玩家。

这样可以做到：

```text
玩家仍然可以在区域内移动；
Camera 固定在设计好的构图中心；
zoom 也由该区域控制；
不会因为玩家贴近世界边界而触发复杂的 limitInWorld 修正。
```

这本质上是“镜头导演系统”的第一步。

### 示例 1：玩家不贴边，zoom = 2

假设：

```text
窗口大小：1600 x 900
玩家世界坐标：1800, 700
zoom = 2
玩家不贴地图边界
不考虑鼠标偏移和平滑滞后
```

可见世界范围：

```text
visibleW = 1600 / 2 = 800
visibleH = 900 / 2 = 450
```

Camera 直接跟随玩家：

```text
centerX = 1800
centerY = 700
```

视口四边：

```text
left   = 1800 - 800 / 2 = 1400
right  = 1800 + 800 / 2 = 2200
bottom = 700 - 450 / 2 = 475
top    = 700 + 450 / 2 = 925
```

玩家屏幕坐标：

```text
screenX = 1600 / 2 + (1800 - 1800) * 2 = 800
screenY = 900 / 2 - (700 - 700) * 2 = 450
```

玩家在屏幕中心。

如果另一个物体在世界坐标 `2000, 700`：

```text
screenX = 800 + (2000 - 1800) * 2 = 1200
screenY = 450 - (700 - 700) * 2 = 450
```

它会出现在玩家右侧 400 像素处。

这个例子最简单，说明的是：

```text
不贴边时，Camera center 基本等于跟随目标坐标。
```

### 示例 2：玩家贴左下角，zoom = 3

假设：

```text
窗口大小：1600 x 900
玩家世界坐标：100, 100
zoom = 3
地图左下角从 0, 0 开始
不考虑鼠标偏移和平滑滞后
```

可见世界范围：

```text
visibleW = 1600 / 3 ≈ 533.33
visibleH = 900 / 3 = 300

halfW ≈ 266.67
halfH = 150
```

如果 Camera 直接跟随玩家：

```text
centerX = 100
centerY = 100
```

这时视口会看到地图外，所以 `limitInWorld()` 会把 Camera 中心修正为：

```text
centerX = 266.67
centerY = 150
```

玩家屏幕坐标：

```text
screenX = 800 + (100 - 266.67) * 3 ≈ 300
screenY = 450 - (100 - 150) * 3 = 600
```

玩家不会在屏幕中心，而是偏向左下方。这不是 bug，而是边界限制生效了。

这里要记住：

```text
Camera 跟随玩家：
    尽量让玩家接近屏幕中心。

Camera 边界限制：
    不允许视口看到世界外。

当两者冲突时：
    边界限制优先。
```

### 示例 3：按住 zoom 时向右走

假设：

```text
窗口大小：1600 x 900
上一帧玩家 X = 1800
当前帧玩家 X = 1820
上一帧 camera.centerX = 1800
当前按住 V，targetZoom = 3
上一帧 zoom = 1
zoomSpeed = 0.08
followSpeed = 0.16
不考虑鼠标偏移和边界
```

先更新 zoom：

```text
zoom = 1 + (3 - 1) * 0.08
zoom = 1.16
```

再计算目标中心：

```text
targetCenterX = 1820
```

Camera 中心平滑靠近：

```text
centerX = 1800 + (1820 - 1800) * 0.16
centerX = 1803.2
```

玩家屏幕 X：

```text
screenX = 800 + (1820 - 1803.2) * 1.16
screenX ≈ 819.49
```

玩家会略微偏右，这是因为 Camera 平滑跟随有滞后，不是因为 zoom 锚点又漂了。

这正是中心点模型想解决的问题：

```text
旧模型：
    zoom 会改变正确的左下角，左下角又要平滑追目标。

新模型：
    zoom 改变视口大小和缩放倍率；
    centerX / centerY 仍然是主锚点；
    视口边界每帧从 center 和 zoom 推导出来。
```

### Camera 与视差背景

视差背景不能直接用旧的视口左边界。

原因是：

```text
viewLeft = centerX - visibleW / 2
visibleW = WINDOW_WIDTH / zoom
```

玩家不动但 zoom 改变时，`viewLeft` 也会变。如果背景用 `viewLeft` 判断相机有没有移动，就会误以为相机平移了，结果背景会跟着缩放漂移。

现在视差参考值不应该简单理解为“直接使用真实 `gCamera.centerX`”。因为 Camera 贴边时，zoom 会改变视口大小，`limitInWorld()` 会重新钳制真实 Camera 中心。如果背景直接读取这个被钳制后的中心点，就会把“缩放导致的相机矫正”误判成“玩家真的横向移动了”。

所以现在做法是引入一个背景视差参考锚点。它可以暂时叫 `parallaxCameraX`，但它不应该被理解成真正负责绘制世界的第二台 Camera。

更准确的名字其实是：

```text
parallaxReferenceX
backgroundScrollX
parallaxScrollX
```

它的职责是表达“背景应该响应的相机横向移动量”，并尽量过滤掉一些不希望背景响应的修正量，例如：

```text
zoom 改变
  -> visibleW / visibleH 改变
  -> limitInWorld 重新钳制真实 Camera center
  -> 真实 Camera center 发生变化
```

所以现在做法可以理解为：

```text
parallaxCameraX / parallaxReferenceX = 背景视差参考锚点
parallaxOriginX = 初始化时的参考锚点
parallaxOffsetX = parallaxCameraX - parallaxOriginX
```

`getParallaxCameraCenterX()` 使用固定参考视口宽度计算横向锚点，不直接使用当前 zoom 下被世界边界钳制后的真实 Camera 中心。

然后这份偏移交给背景逻辑层：

```text
BackgroundManager::updateRuntimeTransforms(parallaxOffsetX)
  -> BackgroundLayer::updateRuntimeTransform(parallaxOffsetX)
  -> 写入 runtimeCenterX / runtimeCenterY
  -> Renderer::drawBackground()
  -> Renderer::drawSprite()
```

这说明：

```text
背景的视差移动应该来自稳定的视差参考锚点，
不要来自受 zoom 和世界边界钳制影响的临时视口数据。
```

### 这几次 Camera bug 给我的提醒

这几天背景绘制相关的问题基本都和 Camera 有关，尤其是我把 Camera 主锚点从“视口左下角”改成“视口中心点”以后，旧公式里很多看起来没问题的东西都会变得不对。

这里要记住一个核心变化：

```text
旧 Camera：
    x / y 本身就是视口左下角。

新 Camera：
    centerX / centerY 才是真正的 Camera 主状态。
    viewLeft / viewBottom / viewRight / viewTop 都是根据 center 和 zoom 临时推导出来的。
```

所以后面只要遇到背景、视口、缩放、鼠标偏移相关的问题，就要先检查这件事：

```text
我现在用的这个值，到底是 Camera 的真实主状态，
还是由主状态和 zoom 推导出来的临时视口边界？
```

#### 1. 不要把 viewLeft 当成真实相机位移

`viewLeft` 的计算依赖 zoom：

```text
viewLeft = centerX - visibleW / 2
visibleW = WINDOW_WIDTH / zoom
```

这意味着即使玩家不动、`centerX` 不变，只要 zoom 改变，`viewLeft` 也会改变。

如果背景视差用 `viewLeft` 判断相机移动，就会出现：

```text
玩家没动
Camera center 没动
只是 zoom 变了
但是背景以为相机横向移动了
```

所以视差背景应该使用 `centerX` 的变化量，而不是 `viewLeft` 的变化量。

#### 2. 不要把背景平移和 Camera zoom 直接绑死

这次遇到的一个明显 bug 是类似这样的公式：

```cpp
screenOriginX = -parallaxOffsetX * cameraZoom * parallaxFactor
```

这里的问题是：

```text
parallaxOffsetX:
    相机中心从初始点移动了多少。

cameraZoom:
    当前相机缩放倍率。
```

如果把它们直接相乘，就会导致：

```text
Camera center 不变
parallaxOffsetX 不变
cameraZoom 改变
最终背景横向偏移也跟着改变
```

于是玩家站在原地缩放时，背景会发生横向滑动。

更合理的理解是：

```text
相机移动：
    影响背景横向平移。

背景层自己的 zoom：
    影响该层绘制尺寸。

cameraZoom：
    不应该额外制造背景横向位移。
```

如果要让背景层的平移跟随自己的缩放关系，应该优先考虑使用该层自己的 `layerZoom`，而不是直接用全局 `cameraZoom` 去放大相机移动量。

#### 3. 越靠地图右侧，错误越明显

这个现象也很重要。

```text
parallaxOffsetX = gCamera.centerX - parallaxOriginX
```

角色越往地图右边走，`gCamera.centerX` 越大，`parallaxOffsetX` 也越大。

如果错误地写成：

```text
parallaxOffsetX * cameraZoom
```

那么 zoom 改变带来的错误偏移会随着 `parallaxOffsetX` 变大而变大。

所以有些 Camera bug 会出现这种表现：

```text
地图左边看起来问题不明显；
走到地图右边以后，一缩放背景就明显滑动。
```

这不是错觉，而是公式里的错误项本身随着世界坐标位置变大了。

#### 4. Debug 面板的价值

这次 debug 面板很有用。只靠肉眼看，很容易怀疑是：

```text
背景图有问题
tile 绘制有问题
角色缩放有问题
Camera center 又漂了
```

但 debug 面板显示：

```text
缩放前后 Camera Center 基本不变；
变化的是 Zoom；
背景却发生横向滑动。
```

这就能判断问题不在 Camera 跟随本身，而是在背景绘制公式里还有某个和 zoom 相关的量参与了横向偏移。

以后遇到类似问题，我应该先看这些数据：

```text
Camera center 是否真的变了？
viewLeft 是否因为 zoom 改了？
parallaxOffsetX 是否稳定？
背景公式里有没有把平移量又乘上 zoom？
```

这个经验后面做前景层、WorldSpaceUI、编辑器视图、不同缩放模式时还会反复用到。

#### 5. 贴边缩放会污染真实 Camera center

后来又遇到一个更隐蔽的问题：只有 Camera 贴边时，缩放才会让背景发生滑动；Camera 不贴边时缩放是正常的。

这个问题和前面“不该用 viewLeft”不完全一样。这里的问题是：

```text
玩家贴近世界边界
  -> zoom 改变视口宽度
  -> Camera 尝试跟随玩家
  -> limitInWorld() 为了不露出世界外区域，重新钳制 Camera center
  -> 真实 gCamera.centerX 发生变化
  -> 背景误以为这是玩家横向移动
```

所以现在背景视差有一个专用参考锚点：

```text
真实 Camera center:
    用于最终 worldToScreen 绘制转换。

视差 Camera center:
    用固定参考视口计算，只表达我希望背景响应的横向移动。
```

这件事让我更明确了一点：Camera 可以负责基础规则，但不要把所有视觉特例都塞进 Camera。背景这种特殊对象应该在自己的逻辑层消化自己的规则，然后再把结果交给通用渲染流程。

#### 6. 贴边缩放问题也可以用关卡设计绕开

这次问题还有一个更重要的提醒：不是所有复杂情况都必须靠数学公式硬修。

如果一个区域天然会让 Camera 频繁贴边，又需要缩放和视差背景，那么可以从设计层避开这个极端状态：

```text
不要让玩家真的走到世界边界；
用墙体、空气墙、门、悬崖或碰撞体限制玩家；
让 Camera 在安全区域内跟随；
需要特殊构图时，用 CameraAnchor / CameraZone 接管镜头。
```

例如玩家进入一个 POI、房间或 Boss 区域时，可以让 Camera 固定到一个没有渲染信息的逻辑锚点，而不是继续跟随玩家：

```text
玩家移动：
    仍然由碰撞和输入控制。

Camera：
    固定到场景中心 / 房间中心 / 演出锚点。

Zoom：
    由这个区域或锚点决定，不再跟随玩家位置频繁变化。
```

这种做法本质上相当于把 `limitInWorld()` 从“主要镜头规则”降级为“最后保险”。

最终比较健康的结构应该是：

```text
1. 玩家边界：用碰撞体和关卡结构限制。
2. 镜头构图：用 CameraMode / CameraAnchor / CameraZone 控制。
3. 技术兜底：用 limitInWorld 防止露出世界外。
4. 背景视差：用 parallax reference 过滤掉不该响应的 Camera 修正。
```

这说明 Camera 不只是坐标转换工具，它也是关卡设计、演出设计和视觉构图的一部分。

---

## Tile 碰撞第一版

现在 tile 碰撞的思路是：

```text
视觉 tile：
    决定地图上画什么。

碰撞 tile：
    决定这个格子有没有碰撞，碰撞盒多大，通行规则是什么。
```

第一版先没有做单独的碰撞编辑文件，而是根据 tileId 自动生成默认碰撞。

这样做的好处是：

- 我不用每次手写一整份 collision map。
- 视觉 tile 和碰撞逻辑能先快速对应起来。
- 以后如果加 `map_collision.txt` 或 JSON，可以把它作为覆盖层。

当前比较适合的规则是：

```text
地面、箱子、砖块、金属块：
    默认 solid。

草地平台：
    默认单向平台。

背景土墙、装饰物、天空：
    默认 none。
```

单向平台目前只处理“从上往下踩住”，也就是横板跳跃里最常用的情况。后面如果需要从左右或下方有特殊规则，可以再扩展通行方向。

---

## 游戏初始化与某一 Tick 的生命周期

为了让我能直观地理解代码运行的顺序，下面梳理了整个项目在两个关键阶段的数据流动过程。

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

我需要额外区分这几件事：

```text
对象存在：
    容器里有这个对象。

对象可见：
    这个对象允许生成或提交 sprite。

对象被绘制：
    Renderer::drawSprite() 真的完成了一次绘制。
```

Debug 面板 Render 区显示的是第三种，不是第一种。

---

## 操作方式

| 按键 | 功能 |
| --- | --- |
| 左方向键 | 向左移动 |
| 右方向键 | 向右移动 |
| 上方向键 | god 模式下向上移动 |
| 下方向键 | god 模式下向下移动 |
| Shift | 冲刺 |
| Space | 跳跃 |
| E | 交互意图预留 |
| 鼠标移动 | 影响镜头观察偏移 |
| 鼠标左键 | 输出鼠标点击位置测试信息 |
| B | 镜头缩小 / 拉远 |
| V | 镜头放大 / 拉近 |
| 1 | 切换输入控制到 0 号实体 |
| 2 | 切换输入控制到 1 号实体 |
| 3 | 切换输入控制到 2 号实体 |
| 4 | 切换输入控制到 3 号实体 |
| F1 | 镜头跟随 0 号实体 |
| F2 | 镜头跟随 1 号实体 |
| F3 | 镜头跟随 2 号实体 |
| F4 | 镜头跟随 3 号实体 |
| W / S / A / D | 切换测试 UI 面板锚点 |
| F5 | 开关实体碰撞框 |
| F6 | 开关 tile 调试碰撞框 |
| F7 | 开关所有可绘制对象的绘制边界框 |
| F8 | 开关 Debug 面板整体显示 |
| F9 | 临时开关 Debug Entity 分区 |
| F10 | 临时开关 Debug Render 分区 |
| F11 | 临时开关 Debug Camera 分区 |
| Esc | 退出程序 |

---

## 当前资源

目前主要资源大概是：

```text
assets/
  tex/
    maps/
      tileset.png
      map.txt
      background.jpg
      Clouds 2/
        1.png
        2.png
        3.png
        4.png
      Clouds 5/
        1.png
        2.png
        3.png
        4.png
        5.png
    entities/
      characters/
        player1_idle_L.png
        player1_idle_R.png
        player1_walk_L.png
        player1_walk_R.png
        player1_run_L.png
        player1_run_R.png
        player1_jump_start_L.png
        player1_jump_start_R.png
        player1_jump_loop_L.png
        player1_jump_loop_R.png
        player1_jump_end_L.png
        player1_jump_end_R.png
        player2.png
        player3.png
        player4.png
      items/
        MonedaD.png
        MonedaP.png
        MonedaR.png
docs/
  camera_zoom_viewport.png
```

以后如果引入 JSON 配置，资源路径就不应该继续大量写在 `Level` 的初始化里。

我希望后面的方向是：

```text
ResourceManager:
    负责外部资源生命周期。

配置文件 / definition:
    描述某类对象需要哪些资源。

Level:
    只负责创建对象和调度，不直接知道太多资源路径。
```

---

## 当前学习重点

现在这个项目最重要的学习点是职责边界和数据流。

我现在应该持续盯住这些问题：

- `Level` 是不是又开始塞业务逻辑了。
- `Entity` 是不是又开始变成超级类了。
- `Animator` 有没有越界去制造玩法状态。
- `Renderer` 有没有把绘制入口统一住。
- `Camera` 的中心点、zoom、视口边界有没有混成一团。
- `limitInWorld()` 有没有被误用成主要镜头设计手段，而不是最后保险。
- `CameraMode / CameraAnchor / CameraZone` 这类镜头导演概念是否需要逐步引入。
- `BackgroundLayer` 的视差/fixed 规则有没有留在自己的逻辑层，而不是又跑回 Renderer 或 Camera。
- `TileMap` 的视觉层和碰撞层有没有被写死绑死。
- `TileInstance` 是否继续保持“可被单独获取和变换”的方向。
- `sprite` 是否继续作为单帧绘制对象的统一中间数据。
- `MovementHandle` 到底更像组件还是系统。

目前比较明确的职责划分：

```text
Level:
    关卡生命周期和调度。

Renderer:
    绘制 sprite、UI、调试框，并统计真实绘制数量。

Camera:
    世界坐标到屏幕坐标的转换、缩放、视口范围。
    `limitInWorld()` 是技术兜底，不应该承担所有镜头构图职责。

ResourceManager:
    资源生命周期。

Animator:
    动画状态选择。

AnimationClip:
    动画资源描述。

AnimationPlayer:
    当前动画片段的播放推进（原 animatedSprite）。

CollisionBox:
    碰撞盒数据和局部到世界的转换。

CollisionManager:
    状态化碰撞与重叠检测，位移阻挡计算与去重分发。

MovementHandle:
    移动、重力和跳跃。

TileMap:
    视觉 tile、TileInstance 和第一版 tile 碰撞数据。

BackgroundManager:
    管理背景层顺序，并在逻辑层更新背景 runtime transform。

BackgroundLayer:
    保存背景层基础位置、runtime 位置、绘制模式和图片数据。
```

最近我对“渲染对象”的理解也更清楚了：

```text
场景中存在一个对象
    不代表它一定会被绘制。

对象有 sprite
    不代表这一帧一定真的画到了屏幕上。

Renderer 统计的数量
    应该是 drawSprite 成功执行后的真实绘制数量。
```

这个区分后面做视口裁剪、trigger object、纯碰撞对象、纯逻辑对象时会很重要。

最近一直在想的长期方向：

```text
轻继承 + 组件化对象 + prefab / definition + Trigger/Event + 内部关卡编辑器
```

我现在还没有真正实现 ECS，也没有真正实现完整组件系统。现在只是把很多功能从 `Entity` 里拆出来了。

这两者不一样：

```text
功能拆分：
    把原来写在 Entity 里的逻辑拆成不同类。

组件化：
    不同实体可以按需拥有不同能力。
```

后面不要把这两个概念混在一起。

---

## 下一步方向

短期可以继续做：

- 停下来整理最近的 sprite / tile / background / UI 重构思路。
- 基于 `getViewLeft/Right/Bottom/Top` 给 tile 和 entity 绘制增加粗视口裁剪。
- 给视口裁剪单独开分支，不要和其他大结构调整混在一起。
- 引入 `map_collision.txt` 或 JSON 配置，作为 tile 默认碰撞规则的覆盖层。
- 清理 `TileMap` 里的默认 tileId 映射表。
- 继续观察 `BackgroundLayer` 是否需要从“层”进一步变成“可动态生成的背景对象”。
- 继续观察背景层是否需要区分 ScreenSpace / ParallaxSpace / WorldSpace。
- 明确 `parallaxCameraX` 更像 parallax reference / background scroll，而不是真正负责绘制的第二台 Camera。
- 研究平铺背景是否应该通过动态生成多个背景对象来实现，而不是只生成多个 sprite。
- 维持并确保 AnimationPlayer 在各个模块里的规范调用。
- 让金币、装饰物、场景交互物拥有更轻量的动画控制。
- 整理 `Entity` 里的 bool 状态，把状态按职责分组。
- 已经完成 game.cpp 的分文件拆分，后续逐步进行更精细的解耦。

长期方向：

- 做出轻量 GameObject / Entity / TriggerObject 层级。
- 形成 prefab / definition 风格的对象创建流程。
- 建立 Trigger / Event / Director 系统。
- 引入 `CameraMode`，区分跟随玩家、固定点、锁定区域和脚本镜头。
- 增加 `CameraAnchor` / `CameraZone`，用于 POI、房间、Boss 区域和剧情构图。
- 让 `limitInWorld()` 逐渐退回为安全兜底，而不是主要镜头设计手段。
- 通过碰撞体 / 空气墙 / 场景结构限制玩家活动范围，减少 Camera 直接贴世界边界的情况。
- 做内部关卡编辑器，减少 Tiled、`map.txt`、代码初始化之间的来回转换。
- 支持运行时创建对象、保存地图、重新加载地图。
- 用数据驱动方式组合不同玩法规则。
- 在横板、top-down、飞行射击等不同玩法模式之间复用底层系统。

---

## 运行环境

当前项目主要在下面环境里运行：

- Windows
- Visual Studio
- EasyX
- C++20

用到的东西包括：

- `graphics.h`
- `windows.h`
- `conio.h`
- `fstream`
- `Msimg32.lib`
- `winmm.lib`

---

## License

代码部分使用 MIT License。

图片、音效、字体等资源如果没有特别说明，就先只当作个人学习测试资源，不一定包含在开源授权范围内。
