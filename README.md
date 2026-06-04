# 基础 C++ 2D 游戏方向练习测试

这是一个个人学习向的 C++ 2D 游戏原型项目。

项目目前主要基于 **EasyX** 进行图形绘制，目标不是马上做成完整游戏，而是通过一个能运行的小项目，逐步理解 2D 游戏里常见的底层模块应该如何组织，例如主循环、输入、实体、动画、碰撞、地图、镜头、资源管理、渲染调度和 UI。

目前它更像是一个 **2D 游戏框架雏形 / 功能实验场**，而不是完整玩法项目。

---

## 当前状态概览

项目已经能运行一个基础 2D 场景：

- EasyX 窗口与主循环
- 背景图绘制
- tile map 读取和绘制
- 玩家动画 idle / walk / run / jumpStart / jumpLoop / jumpEnd 切换
- 基础重力、跳跃、冲刺
- 实体之间的 AABB 阻挡和重叠检测
- 金币拾取事件和音效播放
- 摄像机跟随、缩放和鼠标偏移
- `Animator` 已作为实体动画控制组件接入
- 简单 UI 面板测试
- 实体碰撞框和 tile 碰撞框调试显示开关

最近一段时间主要不是添加新玩法，而是在整理结构。虽然画面变化不大，但内部已经开始从“所有逻辑堆在 Entity 里”逐步拆成多个职责更清晰的模块。

---

## 当前主要模块

### `Level`

`Level` 现在是当前关卡的调度者。

它负责：

- 初始化关卡资源
- 初始化地图、背景、UI、实体设置
- 每帧调度输入、实体更新、相机更新、事件检测、UI 更新
- 持有当前关卡中的核心对象，例如 `TileMap`、`Entity` 数组、`Renderer`、`ResourceManager`

目前 `Level` 仍然还保留一些旧结构，例如实体数组初始化时仍然直接写了部分资源路径。后续希望进一步把实体创建和资源绑定分开。

### `Renderer`

已经抽象出了 `Renderer`，用于统一调度绘制。

目前它负责：

- 绘制背景
- 绘制 tile map
- 绘制实体 sprite
- 根据开关绘制实体碰撞框
- 根据开关绘制 tile 调试碰撞框
- 绘制当前测试 UI

现在 `Level::draw()` 已经不再亲自遍历和绘制所有对象，而是委托给 `Renderer`。

调试开关：

- `F5`：开关实体碰撞框
- `F6`：开关 tile 碰撞框

### `Entity`

`Entity` 目前仍然是项目中最核心的对象，但它已经比早期精简了一些。

现在 `Entity` 主要保有：

- 世界坐标 `x / y`
- 速度和垂直速度
- 是否受控制
- 是否参与重叠检测
- 是否作为阻挡物
- 是否 god 模式
- 存活状态
- 物理状态，例如 `onGround`、`InAir`、`jumping`、`sprinting`
- 碰撞盒组件 `CollisionBox`
- 动画播放对象 `animatedSprite`
- 当前单帧渲染数据 `sprite`
- 朝向状态
- 动画控制组件 `Animator`

已经从 `Entity` 中逐步剥离出去的内容：

- 输入采集交给 `InputManager`
- 玩家输入到行为意图的转换交给 `PlayerController`
- 移动、重力、跳跃、冲刺处理交给 `MovementHandle`
- 碰撞重叠、阻挡位移修正和世界边界限制交给 `CollisionHandle`
- 绘制调度交给 `Renderer`
- 实体 sprite 绘制和实体碰撞框调试绘制交给 `Renderer`
- 碰撞盒世界坐标计算交给 `CollisionBox`
- 动画图片资源开始交给 `ResourceManager` 管理

目前 `Entity` 仍然需要继续拆分的内容：

- 动画状态切换逻辑已经迁移到 `Animator`
- 旧的 `updateAnimationByIntent` 和 `changeAnimation` 已从 `Entity` 中删除
- `animatedSprite` 当前已经更接近动画播放器，后续可以继续改名或拆成 `AnimationPlayer`
- 实体构造函数仍然保留部分旧的图片路径加载逻辑
- 不同类型对象目前仍然用同一个 `Entity` 表示

后续目标是让 `Entity` 更接近一个基础游戏对象容器，而不是所有系统逻辑的集中点。

### `CollisionBox`

`CollisionBox` 已经从单纯的数据结构，变成一个更像碰撞盒组件的小结构。

它负责保存：

- 原始宽高
- 偏移
- 缩放

它也负责把局部碰撞盒转换成世界坐标中的 `RectBox`：

- `toWorldBox(ownerX, ownerY)`

这样 `Entity` 不再亲自计算碰撞盒的 left / right / bottom / top，而是把这部分逻辑委托给自己的 `CollisionBox`。

当前设计理解：

- `Entity` 拥有 `CollisionBox`
- 外部系统如果想修改实体碰撞盒，应该通过 `Entity` 提供的接口修改
- `CollisionBox` 只负责管理自己的数据和局部到世界的转换
- 真正的碰撞检测和移动修正仍然由 `CollisionHandle` 负责

### `CollisionHandle`

`CollisionHandle` 负责底层碰撞检测和阻挡修正。

目前支持：

- AABB 矩形重叠检测
- 一维区间重叠检测
- X 轴允许位移计算
- Y 轴允许位移计算
- 世界边界限制

目前碰撞仍然主要发生在实体之间，以及实体和世界边界之间。  
tile 碰撞接口已经有基础，但尚未接入角色移动。

### `MovementHandle`

`MovementHandle` 负责实体移动和物理更新。

它读取 `BehaviorIntent`，然后处理：

- 左右移动
- 冲刺速度
- 重力
- 跳跃
- 下落速度限制
- 调用 `CollisionHandle` 修正位移
- 更新 `onGround`、`InAir`、`jumping`、`sprinting` 等真实游戏状态

也就是说，`Entity` 保存状态，但移动逻辑本身已经交给 `MovementHandle`。

### `InputManager` 和 `PlayerController`

`InputManager` 负责统一采集输入状态。

它记录：

- 当前帧按键状态
- 上一帧按键状态
- 鼠标位置
- 鼠标左键状态

因此可以判断：

- 按住
- 刚按下
- 刚松开

`PlayerController` 负责把输入转换成 `BehaviorIntent`。

目前玩家行为意图包括：

- `moveX`
- `moveY`
- `wantJump`
- `wantSprint`
- `wantInteract`

这个设计让输入采集和移动执行分离，后续 AI 也可以生成类似的行为意图。

### `ResourceManager`

项目已经开始引入 `ResourceManager`。

当前它负责：

- 在 `Level::init()` 阶段统一加载玩家动画图片资源
- 持有这些 `IMAGE`
- 根据 `AnimationId` 返回对应的 `AnimationClip`

这一步的目的，是避免运行时切换动画时反复 `loadimage`。

目前资源管理仍处于过渡阶段：

- 玩家动画资源已经进入 `ResourceManager`
- 其它实体、金币和部分初始图片路径仍然使用旧方式
- 后续希望把所有实体初始动画资源也迁移到 `ResourceManager`

### `AnimationClip`

`AnimationClip` 目前用于描述一段动画资源。

当前包含：

- `IMAGE* image`
- `frameCount`
- `speed`
- `loop`
- `frameWidth / frameHeight`
- `sourceStartX / sourceStartY`
- `frameSpacingX / frameSpacingY`
- `frameColumns`

它的定位是 **动画资源描述数据**，而不是动画播放器，也不是状态机。

现在它已经可以描述：

- 手动指定单帧宽高
- 指定 sprite sheet 中第一帧的起始裁剪坐标
- 指定横向和纵向帧间距
- 指定一行有多少帧

但这些仍然应该只是“数据描述”。  
真正的每帧播放逻辑目前由 `animatedSprite` 处理，后续可以进一步改名或拆分成更明确的 `AnimationPlayer`。

### `animatedSprite`

`animatedSprite` 是当前的底层序列帧播放器，已经不再直接负责绘制。

它负责：

- 当前帧
- 帧计时
- 播放速度
- 循环控制
- 根据 `AnimationClip` 的帧尺寸、起点、间距和列数计算源图裁剪坐标
- 根据当前帧计算 sprite sheet 中的源图区域
- 把当前帧数据写入 `Entity` 持有的 `sprite`

它已经不再负责：

- 实体绘制
- sprite 缩放和偏移
- 根据实体状态选择动画

现在它既兼容旧的 `load(path, frameCount)`，也支持新的：

- `setClip(AnimationClip clip)`

这让它可以绑定已经由 `ResourceManager` 加载好的图片资源。

### `Animator`

项目已经初步抽出 `Animator`。

当前 `Animator` 已经作为 `Entity` 内部持有的动画控制组件存在。它负责：

- 保存当前动画状态
- 保存上一帧空中状态，用于判断刚落地
- 根据 `BehaviorIntent` 和 `Entity` 真实状态选择动画
- 根据朝向选择左/右动画资源
- 处理 idle / walk / run / jumpStart / jumpLoop / jumpEnd 的切换
- 通过 `ResourceManager` 获取 `AnimationClip`
- 通过 `Entity::setAnimationClip` 将动画片段交给 `animatedSprite`

设计理解：

- `Entity` 保存真实游戏状态，例如是否在地面、是否冲刺、是否存活
- `Animator` 读取这些状态，决定如何表现
- `AnimationClip` 只保存动画资源描述
- `animatedSprite` 只负责播放当前 clip，按 clip 裁剪规则计算当前帧，并把源图矩形同步到 `sprite`
- `sprite` 保存当前帧源图矩形、缩放、偏移和可见性
- `Renderer` 读取 `sprite` 和实体坐标完成最终绘制
- 旧的 `Entity::updateAnimationByIntent` 和 `Entity::changeAnimation` 已清理

---

## 当前动画数据流

目前玩家动画切换的大致流程：

```text
InputManager
  -> PlayerController
  -> BehaviorIntent
  -> MovementHandle
  -> 更新 Entity 的真实状态，例如 sprinting
  -> Entity::updateAnimator(intent, resources)
  -> Animator::update(entity, intent, resources)
  -> Animator::changeAnimation(entity, newState, resources)
  -> getPlayerAnimationId(newState)
  -> ResourceManager::getAnimationClip(animationId)
  -> Entity::setAnimationClip(clip)
  -> animatedSprite::setClip(clip)
       -> 读取 frameWidth / frameHeight
       -> 读取 sourceStartX / sourceStartY
       -> 读取 frameSpacingX / frameSpacingY
       -> 读取 frameColumns
  -> animatedSprite::update()
  -> animatedSprite::writeCurrentFrameTo(renderSprite)
       -> 计算 srcX / srcY / srcW / srcH
  -> Renderer::drawEntities()
  -> Renderer::drawSprite(entity.getSprite(), entity.getX(), entity.getY())
  -> Renderer::drawImageTileAlpha()
```

也就是说，资源加载已经从运行时切换中逐步剥离出来，动画状态切换已经由 `Animator` 接管，动画播放器只计算当前帧源图矩形，实体绘制则由 `Renderer` 读取 `sprite` 数据统一完成。

---

## 一帧 Tick 中发生了什么

当前项目还没有独立的时间系统，主循环里基本把“一帧”当作一个逻辑 tick 使用。

主循环大致是：

```text
main loop
  -> input.update()
  -> level.update(input)
  -> cleardevice()
  -> level.draw()
  -> FlushBatchDraw()
  -> Sleep(16)
```

### Level 更新流程

每一帧进入 `Level::update(input)` 后，大致按这个顺序执行：

```text
Level::update(input)
  -> 处理鼠标左键测试输出
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

其中：

- `clearEntityFrameState()` 清理上一帧的临时状态，例如碰撞反馈、重叠状态、阻挡状态
- `updateEntities()` 负责实体移动、物理、碰撞修正和动画推进
- `handleCameraInput()` 处理 F1 ~ F4 镜头目标切换
- `handleUIInput()` 处理 UI 面板锚点切换
- `handleRendererInput()` 处理 F5 / F6 调试绘制开关
- `updateCamera()` 更新镜头位置、缩放和鼠标偏移
- `updateDebugStates()` 输出状态变化调试信息
- `updateOverlapEvents()` 处理实体之间的重叠事件，例如玩家拾取金币
- `listPanel.update()` 更新 UI 面板平滑移动

### 实体更新流程

`updateEntities(input)` 会遍历当前关卡中的实体。

对每一个存活实体，大致流程是：

```text
for each alive entity
  -> 创建默认 BehaviorIntent
  -> 如果是玩家，由 PlayerController 根据输入生成 intent
  -> MovementHandle::update(...)
       -> 读取 intent
       -> 处理水平移动
       -> 处理冲刺
       -> 处理跳跃
       -> 施加重力
       -> 调用 CollisionHandle 计算允许位移
       -> 写回 Entity 的位置和状态
  -> Entity::updateAnimator(intent, resources)
       -> Animator 读取 Entity 状态和 intent
       -> 判断 idle / walk / run / jumpStart / jumpLoop / jumpEnd
       -> 通过 ResourceManager 获取 AnimationClip
       -> Entity::setAnimationClip(...)
  -> Entity::updateAnimatedSprite()
       -> 推进当前动画帧
```

目前只有 0 号实体会由 `PlayerController` 生成玩家输入意图。其它实体暂时使用默认空意图，因此它们不会主动移动。

### 绘制流程

`Level::draw()` 目前已经委托给 `Renderer`：

```text
Level::draw()
  -> renderer.drawBackground(background)
  -> renderer.drawTileMap(tileMap)
  -> renderer.drawEntities(entitys, ENTITY_COUNT)
  -> renderer.drawUI(listPanel)
```

`Renderer` 负责统一控制绘制顺序和调试绘制开关。

---

## Tick 情景示例

下面记录一些具体情况下，一帧或连续几帧中大概会发生什么。

### 简单情景：玩家向左走

当玩家按住左方向键时：

```text
InputManager
  -> 记录 VK_LEFT 当前按下

PlayerController
  -> intent.moveX = -1

MovementHandle
  -> 判断有水平输入
  -> 当前没有 sprint 时使用普通速度
  -> 计算 wantMoveX = -speed
  -> 调用 CollisionHandle::getAllowedMoveX
  -> 如果左侧没有阻挡，写回 Entity.x

Animator 动画逻辑
  -> inputX < 0
  -> currentFacingDirection = LEFT
  -> sprinting == false
  -> changeAnimation(ANIM_WALK_LEFT, resources)

animatedSprite
  -> 推进 walk left 当前帧

Renderer
  -> 在新的位置绘制玩家
```

这个情景里，输入、移动、动画和绘制都会参与，但不会触发跳跃、金币拾取或调试开关。

### 简单情景：玩家按 F5 关闭碰撞框

当玩家按下 F5 时：

```text
InputManager
  -> 检测到 VK_F5 当前帧刚按下

Level::handleRendererInput
  -> renderer.toggleCollisionBox()

Renderer::drawEntities
  -> showCollisionBox 为 false
  -> 只绘制实体 sprite
  -> 不绘制实体碰撞框
```

这个情景不会影响实体真实状态，只改变调试绘制表现。

### 简单情景：玩家站着不动

当玩家没有水平输入时：

```text
PlayerController
  -> intent.moveX = 0
  -> intent.wantSprint = false

MovementHandle
  -> 水平位移为 0
  -> 继续处理重力和垂直碰撞
  -> 如果已经在地面，会维持落地状态

Animator 动画逻辑
  -> inputX == 0
  -> 根据 currentFacingDirection 选择 idle_L 或 idle_R

Renderer
  -> 绘制待机动画
```

这里 `currentFacingDirection` 很重要。它决定没有输入时角色应该朝左待机还是朝右待机。

### 稍复杂情景：玩家向左走，然后按 Shift 冲刺

当玩家已经按住左方向键，又按住 Shift：

```text
InputManager
  -> VK_LEFT 按下
  -> VK_SHIFT 按下

PlayerController
  -> intent.moveX = -1
  -> intent.wantSprint = true

MovementHandle
  -> 检测到有水平输入
  -> wantSprint = true
  -> 如果实体在地面，允许开始 sprinting
  -> self.sprinting = true
  -> currentSpeed = speed * 2
  -> 计算更大的 wantMoveX
  -> 调用 CollisionHandle 修正水平位移
  -> 写回 Entity.x

Animator 动画逻辑
  -> inputX < 0
  -> currentFacingDirection = LEFT
  -> sprinting == true
  -> changeAnimation(ANIM_RUN_LEFT, resources)

ResourceManager
  -> 返回玩家 run left 的 AnimationClip

animatedSprite
  -> setClip(run left)
  -> 推进奔跑动画帧
```

这里要注意：冲刺状态是实体真实游戏状态，影响移动速度；run left 只是动画表现状态。

### 稍复杂情景：玩家从向左冲刺突然改为向右冲刺

当玩家先处于向左冲刺，然后改按右方向键，并继续按住 Shift：

```text
PlayerController
  -> intent.moveX 从 -1 变为 1
  -> intent.wantSprint = true

MovementHandle
  -> sprinting 仍然为 true
  -> currentSpeed = speed * 2
  -> wantMoveX 变为正数
  -> CollisionHandle 检测右侧阻挡
  -> 写回新的 Entity.x

Animator 动画逻辑
  -> inputX > 0
  -> currentFacingDirection = RIGHT
  -> sprinting == true
  -> Animator::changeAnimation(ANIM_RUN_RIGHT, resources)

Animator::changeAnimation
  -> 如果当前动画不是 ANIM_RUN_RIGHT
  -> 切换 Animator 内部 currentAnimState
  -> 通过 ResourceManager 获取 run right 的 clip
  -> animatedSprite::setClip(run right)
```

如果已经处于 `ANIM_RUN_RIGHT`，`changeAnimation` 会直接返回，避免每帧重复切换动画。

### 稍复杂情景：玩家拾取金币

当玩家碰到金币时：

```text
MovementHandle / CollisionHandle
  -> 正常更新玩家位置

Level::updateOverlapEvents
  -> 遍历实体对
  -> 获取两个实体的 RectBox
  -> CollisionHandle::isRectOverlapping(a, b)
  -> 判断 PLAYER + COIN
  -> 播放 coin_pickup.wav
  -> coin.killEntity()

后续帧
  -> 死亡金币不再 update
  -> Renderer 跳过死亡金币绘制
```

这个流程和阻挡碰撞不同。金币拾取属于“重叠事件”，不会阻止玩家移动，而是在重叠发生后触发游戏事件。

### 稍复杂情景：玩家跳起后落地

当玩家按下 Space：

```text
InputManager
  -> 检测到 VK_SPACE 当前帧刚按下

PlayerController
  -> intent.wantJump = true

MovementHandle
  -> 如果 self.onGround == true
       velocityY = JUMP_SPEED
       onGround = false
       InAir = true
       jumping = true
  -> 每帧 velocityY -= GRAVITY
  -> wantMoveY = velocityY
  -> CollisionHandle::getAllowedMoveY
  -> 写回 Entity.y

落地时
  -> allowedMoveY 和 wantMoveY 不一致
  -> wantMoveY < 0
  -> onGround = true
  -> InAir = false
  -> jumping = false
  -> velocityY = 0
```

目前跳跃动画已经接入 `Animator`。`onGround / InAir / jumping` 会参与判断 `jumpStart / jumpLoop / jumpEnd` 的切换。

### 复杂情景：同一帧里同时发生移动、相机更新、动画切换和调试绘制

例如玩家按住右方向键和 Shift，并按下 F5，同时鼠标偏向屏幕右侧：

```text
input.update()
  -> 记录 VK_RIGHT 按下
  -> 记录 VK_SHIFT 按下
  -> 记录 VK_F5 刚按下
  -> 记录鼠标位置

level.update(input)
  -> clearEntityFrameState()
  -> updateEntities(input)
       -> PlayerController 生成 moveX = 1, wantSprint = true
       -> MovementHandle 更新冲刺移动和碰撞
       -> Animator 将实体动画切换到 run right
       -> animatedSprite 推进当前帧
       -> animatedSprite 把当前帧写入 renderSprite
  -> handleRendererInput(input)
       -> F5 切换实体碰撞框显示
  -> updateCamera(input)
       -> 根据玩家位置和鼠标偏移更新相机
  -> updateDebugStates()
       -> 如果冲刺状态刚开始，输出调试信息
  -> updateOverlapEvents()
       -> 检查是否和金币或普通实体重叠
  -> listPanel.update()

level.draw()
  -> Renderer 绘制背景
  -> Renderer 绘制 tile map
  -> Renderer 绘制实体 sprite
  -> 根据 F5 后的开关决定是否绘制实体碰撞框
  -> Renderer 绘制 UI
```

这个情景能体现当前框架的一个核心特点：  
同一帧里，输入只采集一次，但会分别影响移动、动画、相机、调试绘制和事件检测。

---

## 当前仍未完成的重点

### tile 碰撞

tile map 已经能读取和绘制，也能绘制 solid tile 的调试碰撞框。

但是角色移动仍然没有真正接入 tile 碰撞。

后续希望让 tile 的 solid 信息参与：

- `getAllowedMoveX`
- `getAllowedMoveY`

也就是角色移动时不只检测其它实体，还要检测地图中的固体 tile。

### Entity 类型拆分

目前所有对象仍然统一用 `Entity` 表示。

当前更倾向的方向不是“每一种具体物品都派生成一个类”，而是：

```text
轻继承 + 组件化
```

可能的未来结构：

```text
GameObject
  -> CharacterEntity
  -> InteractiveEntity
  -> TriggerEntity
  -> StaticEntity
```

例如金币不一定需要单独成为 `Coin` 类，它更像是一种 `InteractiveEntity` 的配置。

不同对象可以拥有不同的状态列表和不同的 Animator 配置，而不是所有状态都塞进一个巨大的 `EntityState`。

### Animator

`Animator` 已经初步接入，后续还需要继续整理。

已经接管：

- `currentAnimState`
- `wasInAir`
- `changeAnimation`
- `updateAnimationByIntent`

后续还可以继续做：

- 按实体类型扩展不同的 Animator
- 将人形实体、交互地图元素、装饰元素的动画控制区分开

但它不应该接管实体的真实游戏状态。

例如：

- `sprinting`
- `onGround`
- `InAir`
- `jumping`
- `isAlive`

这些仍然属于 `Entity` 或移动/碰撞系统。  
`Animator` 只读取这些状态，并决定动画表现。

### AnimationPlayer 命名

`animatedSprite` 现在已经基本不再承担绘制职责，也不再保存 sprite 显示缩放和偏移。

它当前更接近：

```text
AnimationPlayer
  -> 持有当前 clip 播放状态
  -> 推进 currentFrame
  -> 根据 AnimationClip 计算源图裁剪矩形
  -> 写入 sprite 的 imageSource / srcX / srcY / srcW / srcH
```

后续可以在独立文件拆分阶段，将 `animatedSprite` 正式改名为 `AnimationPlayer`。

### ResourceManager 继续完善

当前 `ResourceManager` 只接管了玩家动画资源。

后续需要继续迁移：

- `player2`
- `player3`
- `player4`
- 金币动画
- 地图资源
- 背景资源
- 音效资源

最终希望 `Level` 不再到处写资源路径，而是在初始化时统一从资源管理器中获取资源。

### 代码文件拆分

目前大部分内容仍然集中在 `game.cpp` 中。

后续可能拆成：

```text
Camera.h / Camera.cpp
InputManager.h / InputManager.cpp
Entity.h / Entity.cpp
CollisionBox.h
CollisionHandle.h / CollisionHandle.cpp
MovementHandle.h / MovementHandle.cpp
TileMap.h / TileMap.cpp
Renderer.h / Renderer.cpp
ResourceManager.h / ResourceManager.cpp
Animator.h / Animator.cpp
Level.h / Level.cpp
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

## 资源文件说明

当前资源目录大致如下：

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

目前资源管理正在从“代码里直接写路径”迁移到 `ResourceManager`。

---

## 运行环境

- Windows
- Visual Studio
- EasyX
- C++20 项目配置
- Win32 API 部分函数

项目使用到：

- `graphics.h`
- `windows.h`
- `conio.h`
- `fstream`
- `Msimg32.lib`
- `winmm.lib`

其中：

- 透明图片绘制使用 `AlphaBlend`
- 音效播放目前使用 `PlaySoundW`
- 输入检测主要使用 `GetAsyncKeyState`

---

## 当前学习重点

当前项目的重点已经从“让功能跑起来”，逐渐转向“理解游戏框架的职责划分”。

最近主要在整理：

- `Entity` 不应该承担所有逻辑
- `Level` 应该负责关卡调度
- `Renderer` 应该负责绘制调度
- `ResourceManager` 应该负责资源生命周期
- `AnimationClip` 应该只描述动画资源
- `animatedSprite` 应该负责播放当前 clip
- `Animator` 负责动画状态切换
- `CollisionBox` 应该负责碰撞盒数据和局部到世界转换
- `CollisionHandle` 应该负责真正碰撞检测和移动修正

这类结构调整短期内不会让画面变化很多，但会让后续添加功能时更容易维护。

当前比较合理的节奏是：

```text
完成一项结构拆分
然后用一个实际功能验证它
```

接下来比较适合验证当前结构的功能是：

- 接入 tile 碰撞

---

## 未来方向

短期目标：

- 继续完善 `ResourceManager`
- 完善 `Animator` 并删除旧动画切换函数
- 让 `Entity` 构造函数逐步摆脱资源路径
- 接入 tile 碰撞
- 继续清理 `Entity` 中的表现逻辑
- 保持 `Renderer` 对调试绘制的统一控制
- 开始考虑对象分类，例如 `CharacterEntity`、`InteractiveEntity`、`TriggerEntity`
- 逐步把代码拆成多个文件

长期目标：

- 把这个项目整理成一个小型 2D 游戏框架雏形
- 用类似思路重构之前的控制台 RPG
- 尝试制作一个简单的 2D RPG / 魔塔 / 平台跳跃原型
- 继续学习更完整的图形或游戏开发库，例如 SDL、DirectX 等

---

## License

代码部分使用 MIT License。

图片、音效、素材等资源如果没有特别说明，则仅作为个人学习测试使用，不一定包含在开源授权范围内。
