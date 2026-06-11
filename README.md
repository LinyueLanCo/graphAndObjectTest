# EasyX C++ 2D 游戏框架练习项目

这是我用 C++ 和 EasyX 做的一个 2D 游戏练习项目。

这个项目现在不急着做成一个完整游戏，更像是我用来理解游戏框架的实验场。我一边加功能，一边拆结构，一边观察数据到底是怎么流动的。现在我最关心的不是“画面有多完整”，而是这些基础系统以后能不能继续扩展。

目前这个 README 主要是写给我自己看的，用来记录：

- 当前项目已经做到哪里。
- 每个模块大概负责什么。
- 一帧 tick 里到底发生了什么。
- Camera、Entity、TileMap、动画、碰撞这些系统的数据是怎么走的。
- 后面继续重构时不要把现在已经理清的思路忘掉。

---

## 当前状态

现在项目已经支持这些东西：

- EasyX 窗口、主循环和双缓冲绘制。
- 世界坐标和 EasyX 屏幕坐标之间的转换。
- 以 `centerX / centerY` 为主锚点的 2D Camera。
- Camera 平滑跟随、鼠标观察偏移、缩放和世界边界限制。
- 多层视差背景，背景层根据相机中心点的位移产生不同速度的滚动。
- `tileset.png`、`map.txt` 文本地图读取和 tile map 绘制。
- tile 碰撞第一版：根据 tileId 自动生成默认碰撞层，支持完整 solid 和单向平台。
- 玩家 idle / walk / run / jumpStart / jumpLoop / jumpEnd 动画切换。
- 基础横板移动、重力、跳跃和冲刺。
- 数字键切换当前输入控制实体。
- `F1-F4` 切换相机跟随实体。
- 实体之间的 AABB 阻挡、重叠检测和金币拾取。
- `Renderer` 统一调度背景、地图、实体、UI 和调试框绘制。
- `ResourceManager` 管理玩家动画资源和 `AnimationClip`。
- `Animator` 从实体真实状态里读取数据，并决定当前动画状态。
- `vector<Entity>` 管理实体列表和状态缓存。

现在项目还没有真正完成组件化，也还没有拆成多个 `.h/.cpp` 文件。现阶段仍然主要写在单个 `game.cpp` 里，是因为我还在学习结构和数据流，单文件更方便我快速观察上下文。

等这些核心概念再稳定一点，再去拆文件会更稳。

---

## 当前代码排布

目前代码大部分还在 [game.cpp](./game.cpp) 里。现在大致可以按下面这个顺序理解：

```text
game.cpp
├─ 头文件、库链接、全局常量
├─ 枚举与基础数据
│  ├─ AnimationState / AnimationId / AnimationSetId
│  ├─ EntityType / FacingDirection
│  └─ TileId / TileCollisionType
├─ 图像绘制工具
│  ├─ putimage_alpha()
│  └─ putimage_alpha_tile()
├─ Camera
│  ├─ 中心点锚点
│  ├─ 世界坐标和屏幕坐标转换
│  ├─ 视口边界推导
│  └─ 缩放、跟随、鼠标偏移、世界边界限制
├─ AnimationClip / ResourceManager
│  ├─ 动画资源描述
│  └─ 玩家动画资源持久化加载
├─ animatedSprite / Sprite
│  ├─ 当前动画帧推进
│  ├─ sprite sheet 裁剪坐标计算
│  └─ 当前帧渲染数据
├─ RectBox / CollisionBox
│  ├─ AABB 数据
│  └─ 局部碰撞盒转世界碰撞盒
├─ TileMap
│  ├─ 视觉 tile 二维数组
│  ├─ 默认 tile 碰撞层生成
│  ├─ tile 碰撞类型查询
│  └─ tile 调试碰撞框绘制
├─ UIBox / SmoothUIPanel
├─ InputManager / BehaviorIntent / PlayerController
│  ├─ 输入采集
│  └─ 输入转换为行为意图
├─ Animator
│  ├─ 根据实体状态选择动画状态
│  └─ 通过 ResourceManager 切换 AnimationClip
├─ Entity
│  ├─ 位置、速度、状态
│  ├─ CollisionBox
│  ├─ Animator
│  ├─ animatedSprite
│  └─ Sprite
├─ CollisionHandle
│  ├─ 实体碰撞检测
│  ├─ tile 碰撞位移修正
│  └─ 世界边界限制
├─ MovementHandle
│  ├─ 水平移动
│  ├─ 冲刺
│  ├─ 跳跃
│  └─ 重力
├─ Renderer
│  ├─ 多层视差背景绘制
│  ├─ tile map 绘制
│  ├─ entity 绘制
│  └─ debug 绘制
├─ Level
│  ├─ 初始化资源、地图、背景、实体和 UI
│  ├─ update 调度
│  └─ draw 调度
└─ main()
```

后面如果要拆文件，可以基本按这个顺序拆。不要一口气大搬家，先拆最稳定、依赖最清楚的部分。

---

## 当前主要模块

### Level

`Level` 现在是关卡调度层。

它负责初始化资源、地图、背景、实体和 UI，也负责每帧按顺序调度输入、移动、碰撞、动画、相机、事件检测和绘制。

现在它主要持有：

- `TileMap`
- `Renderer`
- `ResourceManager`
- `CollisionHandle`
- `MovementHandle`
- `vector<Entity>`
- 背景图片数组
- 背景视差参考位置
- UI 面板和一些状态缓存

我需要记住一点：`Level` 不应该亲自处理太多业务细节。它更像调度者。

移动交给 `MovementHandle`，碰撞交给 `CollisionHandle`，绘制交给 `Renderer`，动画切换交给 `Animator`。如果以后 `Level` 又开始变胖，就说明又有逻辑该被拆出去了。

### Renderer

`Renderer` 负责统一绘制。

目前它负责：

- 多层视差背景。
- tile map。
- entity sprite。
- entity debug 碰撞框。
- tile debug 碰撞框。
- UI。

我之前一直在调整的方向是：所有真正 `putimage` 相关的东西，尽量都收回到 `Renderer` 里。这样外部对象只提供“我要画什么”，而不是自己直接画。

这是目前比较清楚的一条边界：

```text
Entity / TileMap / UI:
    提供绘制所需的数据。

Renderer:
    决定怎么把这些数据画到屏幕上。
```

### Entity

`Entity` 现在已经比最早精简了很多，但它还不是最终形态。

它目前仍然保存：

- 位置。
- 速度。
- 朝向。
- 存活、落地、跳跃、god 模式等状态。
- `CollisionBox`。
- `Animator`。
- `animatedSprite`。
- `Sprite`。

已经拆出去的部分：

- 碰撞盒计算：`CollisionBox`。
- 动画状态选择：`Animator`。
- 动画资源加载：`ResourceManager`。
- 渲染调度：`Renderer`。
- 碰撞检测和位移修正：`CollisionHandle`。
- 移动、重力、跳跃：`MovementHandle`。

但是现在的 `Entity` 仍然不是严格的组件化结构。它只是“把很多逻辑拆成了对象”，还没有做到“每个实体按需挂载不同组件”。

后面如果继续做 prefab / component / ECS 一类的结构，要继续把这两件事分开：

```text
对象身份：
    这个东西是谁，有什么 id，在哪里，是否存在。

对象能力：
    它能不能移动，能不能碰撞，能不能播放动画，能不能触发事件。
```

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

### AnimationClip / animatedSprite / Sprite

目前这三层还没有完全理顺，但方向已经比较明确。

我希望最后大概是这样：

```text
AnimationClip:
    定义一段动画的数据。
    比如图片资源、帧宽高、帧数量、间隔、播放速度、是否循环。

animatedSprite:
    现在更像 AnimationPlayer。
    负责推进时间，算出当前应该播放第几帧，以及这一帧在 sprite sheet 上的裁剪区域。

Sprite:
    保存最终当前帧的绘制信息。
    比如使用哪张图、裁剪坐标、绘制偏移、逻辑绘制尺寸。

Renderer:
    拿 Sprite 的结果，最终调用 EasyX 绘制。
```

这个方向后面还需要继续清理。尤其是 `animatedSprite` 这个名字已经不太准确，之后可以逐渐改成 `AnimationPlayer`。

### TileMap

`TileMap` 现在负责两类数据：

- 视觉层：地图上每个格子画哪个 tile。
- 碰撞层：地图上每个格子对应什么碰撞规则。

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

### CollisionHandle

`CollisionHandle` 负责碰撞检测和位移修正。

现在它主要处理：

- entity 和 entity 的 AABB 阻挡。
- entity 和 tile 的 solid 阻挡。
- 单向平台。
- 世界边界限制。
- 重叠检测。

这里的关键是：碰撞不只是“有没有碰到”，还包括“碰到之后应该怎么修正位置”。

所以现在 `MovementHandle` 想移动实体时，会先向 `CollisionHandle` 查询允许移动多少距离。

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

updateParallaxCamera()
  -> 记录 gCamera.centerX 给视差背景使用
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

现在做法是：

```text
parallaxCameraX = gCamera.centerX
parallaxOriginX = 初始化时的 gCamera.centerX
parallaxOffsetX = parallaxCameraX - parallaxOriginX
```

然后 `Renderer::drawBackground()` 根据每层的参数算绘制位置：

```text
parallaxFactor 越大，层越靠前，横向移动越明显。
zoomFactor 越大，层越靠前，越明显响应 camera zoom。
```

这说明：

```text
背景的视差移动应该来自 Camera 的真实中心位移，
不要来自受 zoom 影响的视口边界。
```

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

## 一帧 Tick 中发生了什么

这里记录当前一帧大致发生了什么，方便以后改系统时对照。

### 总流程

```text
Level::update(input)
  -> clearEntityFrameState()
  -> handleControlInput(input)
  -> updateEntities(input)
  -> handleCameraInput(input)
  -> handleUIInput(input)
  -> handleRendererInput(input)
  -> updateCamera(input)
  -> updateParallaxCamera()
  -> updateDebugStates()
  -> updateOverlapEvents()
  -> listPanel.update()
```

### 实体更新

```text
for each alive entity
  -> 创建 BehaviorIntent
  -> 当前受控实体由 PlayerController 生成输入意图
  -> MovementHandle::update()
       -> 处理水平移动、冲刺、跳跃、重力
       -> CollisionHandle::getAllowedMoveX()
       -> CollisionHandle::getAllowedMoveY()
       -> 根据碰撞结果修正位置和状态
  -> Entity::updateAnimation()
       -> Animator 读取实体状态
       -> 选择当前 AnimationState
       -> 切换 AnimationClip
       -> animatedSprite 推进当前帧
       -> Sprite 保存当前帧绘制数据
```

### 绘制流程

```text
Level::draw()
  -> 计算 parallaxOffsetX
  -> renderer.drawBackground(backgrounds, 4, parallaxOffsetX, gCamera.zoom)
  -> renderer.drawTileMap(tileMap)
  -> renderer.drawEntities(entitys)
  -> renderer.drawUI(listPanel)
  -> renderer.drawTileDebug(tileMap)
```

这里要记住一点：EasyX 的层级就是绘制顺序。先画的在后面，后画的在前面。

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
- `TileMap` 的视觉层和碰撞层有没有被写死绑死。
- `MovementHandle` 到底更像组件还是系统。

目前比较明确的职责划分：

```text
Level:
    关卡生命周期和调度。

Renderer:
    绘制。

Camera:
    世界坐标到屏幕坐标的转换、缩放、视口范围。

ResourceManager:
    资源生命周期。

Animator:
    动画状态选择。

AnimationClip:
    动画资源描述。

animatedSprite:
    当前动画片段的播放推进。

CollisionBox:
    碰撞盒数据和局部到世界的转换。

CollisionHandle:
    碰撞检测和位移修正。

MovementHandle:
    移动、重力和跳跃。

TileMap:
    视觉 tile 和第一版 tile 碰撞数据。
```

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

- 基于 `getViewLeft/Right/Bottom/Top` 给 tile 和 entity 绘制增加粗视口裁剪。
- 引入 `map_collision.txt` 或 JSON 配置，作为 tile 默认碰撞规则的覆盖层。
- 清理 `TileMap` 里的默认 tileId 映射表。
- 继续把 `animatedSprite` 的命名向 `AnimationPlayer` 过渡。
- 让金币、装饰物、场景交互物拥有更轻量的动画控制。
- 整理 `Entity` 里的 bool 状态，把状态按职责分组。
- 逐步把 `game.cpp` 拆成多个文件。

长期方向：

- 做出轻量 GameObject / Entity / TriggerObject 层级。
- 形成 prefab / definition 风格的对象创建流程。
- 建立 Trigger / Event / Director 系统。
- 做内部关卡编辑器，减少 Tiled、`map.txt`、代码初始化之间的来回转换。
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
