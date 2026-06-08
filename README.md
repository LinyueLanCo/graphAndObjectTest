# 基础 C++ 2D 游戏方向练习测试

这是一个个人学习向的 C++ / EasyX 2D 游戏原型项目。

项目目标不是马上做成完整游戏，而是通过一个能运行、能不断重构的小项目，理解 2D 游戏中常见系统的组织方式：主循环、输入、实体、动画、碰撞、地图、镜头、资源管理、渲染调度和 UI。

当前项目更像是一个 **2D 游戏框架雏形 / 功能实验场**。最近的重点是把早期堆在 `Entity` 里的逻辑逐步拆出来，并通过实际功能验证结构是否合理。

---

## 当前状态

当前项目已经支持：

- EasyX 窗口、主循环和双缓冲绘制。
- 背景图、tileset、tile map 文本地图读取与绘制。
- 玩家 idle / walk / run / jumpStart / jumpLoop / jumpEnd 动画切换。
- 基础横板移动、重力、跳跃和冲刺。
- 实体之间的 AABB 阻挡、重叠检测和金币拾取。
- 摄像机跟随、缩放和鼠标偏移。
- `Renderer` 统一调度背景、地图、实体、UI 和调试框绘制。
- `ResourceManager` 管理玩家动画资源和 `AnimationClip`。
- `Animator` 从实体真实状态中读取数据并切换动画。
- `vector<Entity>` 管理实体列表和状态缓存。
- tile 碰撞第一版：根据 tileId 生成默认碰撞层，支持完整 solid 和单向平台。

---

## 当前代码排布

目前代码大部分仍集中在 [game.cpp](./game.cpp) 中，还没有正式拆分到多个 `.h/.cpp` 文件。当前大致排布如下：

```text
game.cpp
├─ 头文件、库链接、全局常量
├─ 枚举与基础数据
│  ├─ AnimationState / AnimationId / AnimationSetId
│  ├─ EntityType / FacingDirection
│  └─ TileId / TileCollisionType
├─ 图像绘制工具
│  └─ putimage_alpha_tile()
├─ Camera
│  ├─ 世界坐标和屏幕坐标转换
│  └─ 缩放、跟随、鼠标偏移
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
│  ├─ 背景绘制
│  ├─ tile map 绘制
│  ├─ entity 绘制
│  └─ debug 绘制
├─ Level
│  ├─ 初始化资源、地图、实体和 UI
│  ├─ update 调度
│  └─ draw 调度
└─ main()
```

后续拆分文件时，可以按这个顺序逐步拆出模块，而不是一次性大搬家。

---

## 当前主要模块

### Level

`Level` 是当前关卡调度者，负责初始化资源、地图、实体和 UI，并在每帧调度输入、移动、碰撞、动画、相机、事件检测和绘制。

目前 `Level` 已经不再亲自处理大量底层绘制和碰撞计算，但它仍然持有 `TileMap`、`Renderer`、`ResourceManager`、`CollisionHandle`、`MovementHandle` 和 `vector<Entity>`。

### Renderer

`Renderer` 负责统一绘制：

- 背景图。
- tile map。
- 实体 sprite。
- 实体 debug 碰撞框。
- tile debug 碰撞框。
- 当前测试 UI。

当前实体 sprite 和碰撞框绘制已经从 `Entity` 迁移到 `Renderer`，避免了实体自己负责绘制自己的旧结构。

### Entity

`Entity` 现在仍然是项目里最核心的对象，但已经比早期精简很多。

当前它主要保存：

- 世界坐标、速度和垂直速度。
- 存活、阻挡、重叠、god 模式等状态。
- `onGround`、`InAir`、`jumping`、`sprinting` 等移动状态。
- `CollisionBox`。
- `Animator`。
- `animatedSprite`。
- 当前帧 `Sprite`。

已经从 `Entity` 中拆出的职责包括：

- 输入采集：`InputManager`。
- 输入到行为意图转换：`PlayerController`。
- 移动和重力：`MovementHandle`。
- 碰撞检测和位移修正：`CollisionHandle`。
- 动画状态切换：`Animator`。
- 动画资源加载：`ResourceManager`。
- 渲染调度：`Renderer`。

当前 `Entity` 仍然还不是严格组件化结构。后续如果要做类似 prefab / 组件挂载的对象系统，需要继续把“对象身份”和“对象能力”分开。

### Animator

`Animator` 已经作为实体动画控制组件接入。

它负责：

- 保存当前动画状态。
- 根据 `Entity` 的真实状态和 `BehaviorIntent` 选择动画。
- 处理 idle / walk / run / jumpStart / jumpLoop / jumpEnd 的切换。
- 根据朝向选择左右动画。
- 通过 `ResourceManager` 获取 `AnimationClip`。

它不负责制造真实游戏状态。比如朝向、速度、是否落地、是否跳跃这些状态仍然属于移动和实体逻辑，`Animator` 只是读取这些状态并决定表现。

### AnimationClip / animatedSprite / Sprite

当前动画数据流是：

```text
Animator
  -> AnimationState
  -> ResourceManager::getAnimationClip()
  -> Entity::setAnimationClip()
  -> animatedSprite::setClip()
  -> animatedSprite::update()
  -> animatedSprite::writeCurrentFrameTo(Sprite)
  -> Renderer::drawSprite()
```

其中：

- `AnimationClip` 描述动画资源和裁剪规则。
- `animatedSprite` 更接近动画播放器，负责帧推进和源图裁剪坐标计算。
- `Sprite` 保存当前帧渲染数据，包括 imageSource、源图裁剪区域、缩放、偏移和可见性。
- `Renderer` 读取 `Sprite` 并执行最终绘制。

### TileMap

`TileMap` 现在不只负责绘制，也拥有第一版 tile 碰撞数据。

当前支持：

- 从 `map.txt` 读取视觉 tile id。
- 根据视觉 tile id 自动生成默认碰撞层。
- 查询每个 tile 的 `TileCollisionType`。
- 根据碰撞类型计算实际世界碰撞盒。
- 绘制 tile debug 碰撞框。

当前碰撞类型：

```cpp
TILE_COLLISION_NONE
TILE_COLLISION_FULL_SOLID
TILE_COLLISION_FULL_ONE_WAY
TILE_COLLISION_TOP_HALF_ONE_WAY
TILE_COLLISION_TOP_LEFT_HALF_ONE_WAY
TILE_COLLISION_TOP_RIGHT_HALF_ONE_WAY
```

其中 one-way 平台不阻挡左右移动，只在实体向下落时产生垂直阻挡。

### CollisionHandle

`CollisionHandle` 负责：

- AABB 矩形重叠检测。
- 一维区间重叠检测。
- 实体之间的 X/Y 轴允许位移计算。
- 实体和 tile 之间的 X/Y 轴允许位移计算。
- 世界边界限制。

当前 tile 碰撞已经接入 `getAllowedMoveX()` 和 `getAllowedMoveY()`。

### MovementHandle

`MovementHandle` 负责：

- 行为意图读取。
- 水平移动。
- 冲刺。
- 跳跃。
- 重力。
- 调用 `CollisionHandle` 修正位移。
- 写回 `Entity` 的位置和移动状态。

`Entity` 保存状态，但移动计算不再直接写在 `Entity` 中。

---

## Tile 碰撞第一版

当前 tile 碰撞采用的是：

```text
视觉 tileId -> 默认碰撞类型 -> collisionTiles[row][col] -> 实际碰撞盒
```

它不是严格独立的手绘 collision layer，但已经摆脱了早期“非空 tile 全部 solid”的做法。

当前规则：

- 土墙主体可以作为背景，不一定阻挡左右。
- 草坪平台可以是顶部半格 one-way 平台。
- 平台边缘可以有左上半格或右上半格碰撞盒。
- 箱子、砖块、金属块等暂时按完整 solid 处理。

当前 `TileMap::getTileCollisionWorldBox()` 会根据 tile 碰撞类型，从完整 tile box 中裁剪出真实碰撞盒：

```text
TOP_HALF：
  bottom = tileTop - tileHeight * 0.5
  top = tileTop

TOP_LEFT_HALF：
  right = tileLeft + tileWidth * 0.5
  bottom = tileTop - tileHeight * 0.5
  top = tileTop

TOP_RIGHT_HALF：
  left = tileLeft + tileWidth * 0.5
  bottom = tileTop - tileHeight * 0.5
  top = tileTop
```

目前这是第一版实现。后续如果同一个 tileId 在不同位置需要不同碰撞规则，就应该引入 `map_collision.txt` 或 JSON 配置作为手动覆盖层。

---

## 一帧 Tick 中发生了什么

当前主循环大致是：

```text
main loop
  -> input.update()
  -> level.update(input)
  -> cleardevice()
  -> level.draw()
  -> FlushBatchDraw()
  -> Sleep(16)
```

`Level::update(input)` 大致流程：

```text
Level::update(input)
  -> clearEntityFrameState()
  -> updateEntities(input)
  -> handleCameraInput(input)
  -> handleUIInput(input)
  -> handleRendererInput(input)
  -> updateCamera(input)
  -> updateDebugStates()
  -> updateOverlapEvents()
  -> listPanel.update()
```

单个实体更新大致流程：

```text
for each alive entity
  -> 创建 BehaviorIntent
  -> 玩家实体由 PlayerController 生成输入意图
  -> MovementHandle::update()
       -> 处理水平移动、冲刺、跳跃、重力
       -> CollisionHandle::getAllowedMoveX()
       -> CollisionHandle::getAllowedMoveY()
       -> 写回实体位置和移动状态
  -> Entity::updateAnimator()
       -> Animator 根据真实状态切换动画
  -> Entity::updateAnimatedSprite()
       -> 推进当前动画帧
```

绘制流程：

```text
Level::draw()
  -> renderer.drawBackground(background)
  -> renderer.drawTileMap(tileMap)
  -> renderer.drawEntities(entitys)
  -> renderer.drawUI(listPanel)
```

---

## 操作方式

| 按键 / 操作 | 功能 |
| --- | --- |
| 左方向键 | 向左移动 |
| 右方向键 | 向右移动 |
| Shift | 冲刺 |
| Space | 跳跃 |
| 鼠标移动 | 影响镜头观察偏移 |
| 鼠标左键 | 输出鼠标点击位置测试信息 |
| B | 镜头缩小 / 拉远 |
| V | 镜头放大 / 拉近 |
| F1 | 镜头跟随 0 号实体 |
| F2 | 镜头跟随 1 号实体 |
| F3 | 镜头跟随 2 号实体 |
| F4 | 镜头跟随 3 号实体 |
| F5 | 开关实体碰撞框 |
| F6 | 开关 tile 调试碰撞框 |
| Esc | 退出程序 |

---

## 当前资源

```text
assets/
  tex/
    maps/
      background.jpg
      tileset.png
      map.txt
    entities/
      characters/
        player1_idle_L.png
        player1_idle_R.png
        player1_walk_L.png
        player1_walk_R.png
        player1_run_L.png
        player1_run_R.png
        player1_jumpStart_L.png
        player1_jumpStart_R.png
        player1_jumpLoop_L.png
        player1_jumpLoop_R.png
        player1_jumpEnd_L.png
        player1_jumpEnd_R.png
        player2.png
        player3.png
        player4.png
      items/
        MonedaD.png
        MonedaP.png
        MonedaR.png
  sound/
    entities/
      item/
        coin_pickup.wav
```

`json.hpp` 已经加入项目目录，作为后续读取 JSON 配置文件的准备，但当前尚未正式接入配置读取流程。

---

## 当前学习重点

当前项目的重点是理解游戏框架中的职责边界：

- `Level` 负责关卡生命周期和调度。
- `Renderer` 负责绘制。
- `ResourceManager` 负责资源生命周期。
- `Animator` 负责动画状态切换。
- `AnimationClip` 负责动画资源描述。
- `animatedSprite` 负责播放当前动画片段。
- `CollisionBox` 负责碰撞盒数据和局部到世界的转换。
- `CollisionHandle` 负责碰撞检测和位移修正。
- `MovementHandle` 负责移动、重力和跳跃。
- `TileMap` 负责视觉 tile 和第一版 tile 碰撞数据。

最近讨论过的长期方向是：

```text
轻继承 + 组件化对象 + prefab / definition
```

当前还没有真正实现 ECS 或完整组件系统，但已经意识到继续堆超级 `Entity` 会带来维护问题。

---

## 下一步方向

短期可以继续做：

- 引入 `map_collision.txt` 或 JSON 配置，作为 tile 默认碰撞规则的覆盖层。
- 清理 `TileMap` 中的默认 tileId 映射表。
- 继续把 `animatedSprite` 的命名向 `AnimationPlayer` 过渡。
- 让金币、装饰物、场景交互物拥有更轻量的动画控制。
- 整理 `Entity` 中的 bool 状态，把状态按职责分组。
- 逐步把 `game.cpp` 拆成多个文件。

长期方向：

- 做出轻量 GameObject / Entity / TriggerObject 层级。
- 形成 prefab / definition 风格的对象创建流程。
- 用数据驱动方式组合不同玩法规则。
- 在横板、top-down、飞行射击等不同玩法模式之间复用底层系统。

---

## 运行环境

- Windows
- Visual Studio
- EasyX
- C++20

项目使用到：

- `graphics.h`
- `windows.h`
- `conio.h`
- `fstream`
- `Msimg32.lib`
- `winmm.lib`

---

## License

代码部分使用 MIT License。

图片、音效、字体等资源如果没有特别说明，则仅作为个人学习测试使用，不一定包含在开源授权范围内。
