# EasyX C++ 2D 游戏框架练习项目

这是我用 C++ 和 EasyX 做的一个 2D 游戏练习项目。

这个项目现在不急着做成一个完整游戏，更像是我用来理解游戏框架的实验场。我一边加功能，一边拆结构，一边观察数据到底是怎么流动的。

## 当前状态

现在项目已经支持这些东西：

- EasyX 窗口、主循环和双缓冲绘制。
- 世界坐标和 EasyX 屏幕坐标之间的转换。
- 以 `centerX / centerY` 为主锚点的 2D Camera。
- Camera 平滑跟随、鼠标观察偏移、缩放和世界边界限制。
- 多层背景系统，背景由 `BackgroundLayer / BackgroundManager` 管理，并按 `renderOrder` 映射为 `zIndex` 决定绘制顺序。
- 稳定排序的背景排布，支持使用负数 Z-Index 实现远景（天空、云、近景树）始终处于瓦片之下，正数 Z-Index 实现近景前景（如海洋）遮挡玩家和地图瓦片。
- 前景图层支持超相机速度视差（`parallaxFactor > 1.0`），让场景极具深度感。
- 背景支持横向平铺、单张世界背景、固定相机背景三种模式。
- `tileset.png`、`map.txt` 文本地图读取和 tile map 绘制。
- tile 碰撞第一版：根据 tileId 自动生成默认碰撞层，支持完整 solid 和单向平台。
- tile 已经从“二维数组直接绘制”推进到 `TileInstance`，每个 tile 实例拥有中心点、offset、scale 和绘制尺寸。
- 玩家 idle / walk / run / jumpStart / jumpLoop / jumpEnd 动画切换。
- 基础横板移动、重力、跳跃和冲刺。
- 数字键切换当前输入控制实体。
- `F1-F4` 切换相机跟随实体。
- 实体之间的 AABB 阻挡、重叠检测和金币/水果拾取。
- `Renderer` 统一调度背景、地图、实体、UI 和调试框绘制。
- `sprite` 成为实体、tile、背景这类单帧绘制对象的统一中间数据。
- Debug 面板可以显示当前帧真实被 `Renderer` 绘制成功的 background / tile / entity sprite 数量。
- `ResourceManager` 管理所有音频、贴图、文本和关卡资源，包括 `AnimationClip` 动画剪辑的加载与解析。
- `Animator` 配合 JSON 规则从实体状态字典读取条件，进行动作状态机状态过渡与切换。
- `EntityManager` 管理实体列表和双索引缓存，对象池连续存放，并支持帧尾延迟安全生成实体。
- 完善的像素对话框 `DialogueBox`：继承自 `UIElement`，实现了打字机特效（可按键跳过并提前计算最终拉伸高度）、文本自动根据宽度和字体大小折行、对话框高度根据文本高度平滑拉伸且自适应中心点/锚点移动。
- `LocalizationManager` 实现多语言本地化配置读取与动态加载。
- `TimerManager` 通用轮询计时器系统，支持关卡级帧数倒计时事件脚本管理。
- **全项目源码编码统一**：所有 `.h`, `.cpp`, `.json` 文件均统一转换为 **UTF-8 with BOM (Code Page 65001)**，彻底消除了 MSVC 在中文环境下的 C4819 字符警告和吞换行符的问题。

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
├─ Image2D.h / .cpp              // 图像资源包装类，保存 EasyX IMAGE 及图片的宽高规格
├─ DialogueBox.h / .cpp          // 对话框 UI 元素类，继承自 UIElement，支持打字机效果、高度自动拉伸与排版折行
├─ LocalizationManager.h / .cpp  // 本地化语言包管理器，负责加载 json 语言文件并获取翻译文本
├─ TimerManager.h / .cpp         // 通用帧计时器管理器，驱动倒计时并在倒计时结束时通知查询者
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

项目所有核心类都已迁移到了独立的 `.h/.cpp` 文件中。下面详细介绍当前项目包含的所有类/结构体及其具体功能、关键成员和核心接口：

---

### 1. 关卡核心调度模块

#### [Level](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/Level.h)
* **核心职责**：关卡的中央调度器与管理者，负责协调初始化资源载入、世界地图构建、实体生命周期、背景图层、UI 元素、调试面板的更新与渲染管线。
* **主要成员**：
  * 地图与渲染：`TileMap tileMap`、`Renderer renderer`、`RenderQueue renderQueue`、`RenderFrameStats renderFrameStats`。
  * 管理器：`ResourceManager resources`、`EntityManager entityManager`、`CollisionManager collisionManager`、`BackgroundManager backgroundManager`、`UIManager uiManager`。
  * 玩法组件：`MovementHandle movementHandle`、`PlayerController playerController`、`DialogueBox dialogueBox`、`LocalizationManager localizationManager`、`TimerManager timerManager`、`LevelDebugger levelDebugger`。
  * 状态：`double worldWidth, worldHeight` 关卡尺寸，`EntityID controlledPlayerId` 当前受控实体ID。
* **核心接口**：
  * `init()`：触发整个关卡资源的预加载、地图读取、实体生成和UI装配。
  * `update(InputManager& input)`：调度一帧内所有的输入捕获、物理移动、碰撞检测、镜头跟随、时间推进、UI缓动等逻辑。
  * `draw()`：清空渲染队列，收集背景、地图及实体的精灵数据，进行层级稳定排序，并分发给渲染器渲染，最后画出碰撞辅助框与顶层对话框。

---

### 2. 渲染与精灵流水线模块

#### [Renderer](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/Renderer.h)
* **核心职责**：底层的 2D 绘图接口封装，实现逻辑坐标与屏幕坐标的自动转换，负责具体把精灵、UI和调试框绘制到 EasyX 双缓冲上，与业务逻辑完全解耦。
* **主要成员**：
  * 控制开关：`bool showCollisionBox`、`bool showTileCollisionBox`、`bool showSpriteBorder` 是否显示调试边界。
  * 字体参数等绘制缓冲。
* **核心接口**：
  * `drawSprite(const sprite& spr)`：将传入的精灵数据根据相机视口缩放与平移，在对应屏幕位置绘制源图的裁剪部分（支持透明度混合与单色填充）。
  * `drawDialogueBox(const DialogueBox& db)`：在屏幕最顶层渲染像素对话框背景与逐字打字机文本。
  * `drawEntityCollisionBoxes() / drawTileCollisionBoxes()`：负责在场景中叠画红色/绿色的物理包围盒。

#### [RenderQueue](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/RenderQueue.h)
* **核心职责**：精灵渲染缓冲队列，用于收集单帧中所有的精灵，在最终渲染前对其进行统一排序以解决绘制遮挡问题。
* **主要成员**：
  * `std::vector<sprite> items`：本帧所有被提交的待画精灵列表。
* **核心接口**：
  * `clear()`：清空队列，准备收集新一帧的精灵。
  * `submit(const sprite& spr)`：将一个精灵的副本压入队列。
  * `sort()`：调用 `std::stable_sort` 对 `items` 按 `zIndex`（渲染深度）由小到大进行稳定排序，保证深度相同的精灵保留原有的提交顺序。
  * `drawAll(Renderer& renderer, RenderFrameStats& stats)`：遍历排好序的队列，分发给 `Renderer::drawSprite` 实际绘制，并将数量统计写入 `RenderFrameStats`。

#### [Sprite](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/Sprite.h)
* **核心职责**：渲染精灵单帧数据的中间载体结构体（所有可见元素如背景、瓦片、实体最终均转换为 `Sprite` 交付队列）。
* **主要成员**：
  * 源数据：`Image2D* texture` 贴图指针，`int srcX, srcY, srcW, srcH` 纹理裁剪区域。
  * 变换属性：`double scaleX, scaleY` 缩放倍率，`double offsetX, offsetY` 渲染中心相对实体位置的偏移量，`int zIndex` 渲染图层深度。
  * 世界坐标数据：`double worldCenterX, worldCenterY, worldDrawW, worldDrawH`（自动在逻辑层算出）。
* **核心接口**：
  * `setSource()`：设置源贴图及裁剪框尺寸。
  * `setWorldDrawData()`：设定世界坐标下的绘制矩形中心与大小。

---

### 3. 实体系统模块

#### [Entity](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/Entity.h)
* **核心职责**：游戏实体数据容器与组件持有者。维护实体的空间位置、朝向和运动状态，持有碰撞盒、状态机及动画播放器等组件。
* **主要成员**：
  * 基本信息：`EntityID instanceId`，`std::string name`，`EntityType entityType`。
  * 空间与速度：`double x, y`（中心世界坐标），`double speed`，`double velocityY`，`facingDirection currentFacingDirection`。
  * 物理状态位：`bool controlled`（受控），`bool InAir`，`bool onGround`，`bool sprinting`，`bool jumping`，`bool isAlive`。
  * 组件：`CollisionBox collisionBox`，`Animator animator`，`AnimationPlayer animation`，`sprite renderSprite`。
  * 重叠交互：`std::vector<OverlapInfo> currentOverlaps` 本帧重叠列表，`bool flagActivatedJustNow` 旗帜升顶触发事件标记。
* **核心接口**：
  * `updateAnimator(BehaviorIntent intent)`：触发持有组件 `Animator` 执行状态匹配逻辑。
  * `updateAnimatedSprite()`：推进 `AnimationPlayer` 播放时间并将裁剪数据写入 `renderSprite`，同时处理金币吃完销毁、旗杆触发等自治状态。
  * `resolveOverlaps(EntityManager& entityManager)`：实体根据自身重叠列表决定动作响应，如玩家捡金币、触发旗杆动画等。

#### [EntityManager](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/EntityManager.h)
* **核心职责**：对象池设计。预分配连续内存空间管理全部实体实例，维护活跃和死亡索引，保证迭代与动态生成的高效安全。
* **主要成员**：
  * 池容器：`std::vector<Entity> entities`（固定分配 200 个槽位）。
  * 索引：`std::vector<size_t> activeIndices` 活跃实体槽下标，`std::vector<size_t> deadIndices` 死亡复用槽下标。
  * 缓存队列：`std::vector<SpawnRequest> spawnQueue` 本帧延迟生成实体请求队列。
  * 配置表：`std::unordered_map<std::string, EntityTemplate> templates` 从 JSON 读入的实体模板。
* **核心接口**：
  * `loadTemplates()` / `loadEntities()`：从 JSON 读取实体属性模板和关卡初始摆放实体。
  * `queueSpawnEntity(double x, double y, const std::string& templateName)`：在关卡事件中申请动态生成实体，本帧暂存队列中。
  * `processSpawns(AnimationClipManager& animClips)`：在帧末安全执行，销毁已死实体并将死索引回收，从死索引中复活并初始化新生成的实体，彻底规避了迭代中修改活跃容器导致崩溃的问题。

---

### 4. 动画状态机与资源模块

#### [AnimationClip](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/AnimationClip.h)
* **核心职责**：描述一个动画片段的源数据配置结构体。
* **主要成员**：
  * `Image2D* image`：源精灵图贴图指针。
  * `int sourceStartX, sourceStartY`：裁剪区起始像素坐标。
  * `int frameWidth, frameHeight`：单帧宽高像素值。
  * `int frameCount, frameColumns`：动画总帧数与序列帧列数。
  * `int speed`：帧率速度（每隔几帧切换下一序列帧），`bool loop` 是否循环。

#### [AnimationClipManager](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/AnimationClipManager.h)
* **核心职责**：全局动画剪辑资源库，负责从外部 JSON 配置文件中载入并缓存所有实体的动画序列数据。
* **主要成员**：
  * `std::unordered_map<std::string, AnimationClip> clips`：动画剪辑名到片段数据的哈希配置表。
* **核心接口**：
  * `init(const std::string& jsonPath, ResourceManager& res)`：解析 animations.json 并拉取纹理对象指针存入剪辑库。

#### [AnimationPlayer](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/AnimationPlayer.h) (原 `animatedSprite`)
* **核心职责**：动画播放控制器。负责驱动单个实体当前片段的时间轴推进，计算本帧裁剪框并将其写入对应的 `sprite` 供渲染器绘制。
* **主要成员**：
  * 运行状态：`int currentFrame` 当前序列帧号，`int frameTimer` 时间计数器，`bool isPlaying`，`bool isLoop`。
  * 裁剪定位：`int sourceStartX, sourceStartY`，`int frameWidth, frameHeight`，`int frameSpacingX, frameSpacingY`，`int frameColumns`。
* **核心接口**：
  * `setClip(AnimationClip clip)`：给当前播放器指定新的片段并重置播放状态。
  * `update()`：每帧被调用，计数器加 1，达到 `frameInterval` 时切换到下一序列帧，处理循环/非循环终点。
  * `writeCurrentFrameTo(sprite& targetSprite)`：换算当前帧在贴图上的二维行列位置，算出精确的裁剪矩形起点并写入目标精灵中。

#### [Animator](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/Animator.h)
* **核心职责**：实体动画状态机。基于当前实体的物理物理状态与传入的行为意图，决策实体当前应当处于什么动画状态，并在变化时更新播放器片段。
* **主要成员**：
  * 状态：`std::string currentState` 当前状态名（如 "idle", "run", "jump"）。
  * 转换表：`std::vector<TransitionRule> transitionRules` 动作过渡条件规则。
  * 映射表：`std::unordered_map<std::string, std::string> stateToClipName` 状态与对应动画资源片段命名的映射关系。
* **核心接口**：
  * `update(Entity& owner, BehaviorIntent intent)`：读取 owner 实体上的运动变量（如水平速度、是否在空中等），在过渡规则中扫描满足条件的目标状态，若发生突变，则通过 `owner.getAnimation().setClip` 将 owner 的片段切为新状态对应的片段。

---

### 5. 运动与碰撞逻辑模块

#### [MovementHandle](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/MovementHandle.h)
* **核心职责**：运动物理模拟系统。根据实体的移动意图和自身物理属性，计算重力、起跳初速度、冲刺速度，并结合碰撞检测更新实体的坐标。
* **核心接口**：
  * `update(...)`：处理物理更新的核心入口：
    1. 意图解析：当 `intent.wantJump` 且 `owner.onGround` 时赋予起跳瞬时向上速度，重置落地标记。
    2. 摩擦力/速度计算：若按住 `wantSprint` 则倍增水平速度，结合惯性衰减计算期望位移。
    3. 水平碰撞过滤：通过 `collisionManager.getAllowedMoveX` 探测前路 solid 格子或阻挡实体，获得修正后的安全位移，更新 X 坐标。
    4. 重力累加：速度向下递减。
    5. 垂直碰撞过滤：通过 `collisionManager.getAllowedMoveY` 探测头顶 solid 瓦片或脚底单向平台，获得安全位移，更新 Y 坐标。

#### [CollisionManager](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/CollisionManager.h) (原 `CollisionHandle`)
* **核心职责**：碰撞与重叠检测的核心处理器，计算物理位移阻挡限制，并分发状态去重的重叠 overlap 事件。
* **主要成员**：
  * 状态缓存：`std::unordered_set<std::string> lastOverlapPairs` 上一帧发生重叠的实体对唯一键集合，用于去重；`std::vector<OverlapPair> currentOverlapPairs` 本帧重叠实体对。
* **核心接口**：
  * `getAllowedMoveX() / getAllowedMoveY()`：使用 AABB 轴投影探测。将实体的 CollisionBox 沿对应轴延伸期望位移值，遍历世界中所有 solid 瓦片和阻挡实体做相交测试，截断并返回最大允许安全滑动距离。
  * `updateOverlapEvents(EntityManager& em)`：在所有活跃实体间做两两 overlap 包围盒测试，若有相交且它们不是阻挡关系（例如玩家和金币/旗杆），则将对方登记在 `currentOverlaps` 中。对比 `lastOverlapPairs` 记录新进入的碰撞对并打印日志，完成帧尾对历史缓存的拷贝。
  * `resolveEntityOverlaps(EntityManager& em)`：调度所有发生重叠的活跃实体，执行它们自身的自治反馈函数 `resolveOverlaps`。

#### [Collision](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/Collision.h) (包含 `RectBox` 与 `CollisionBox`)
* **核心职责**：AABB 包围盒几何定义与局部坐标变换。
* **主要成员**：
  * `RectBox`：包含实体的世界空间左下角 `x, y` 及宽高 `w, h` 的轴对齐包围盒。
  * `CollisionBox`：定义实体自身的碰撞盒宽高 `w, h` 以及相对实体原点中心点的局部偏移量 `offsetX, offsetY`。
* **核心接口**：
  * `CollisionBox::getWorldBox(double entityX, double entityY)`：将相对偏移坐标与实体的世界中心坐标进行相加，转换生成正确的世界空间 `RectBox` 包围盒。

---

### 6. 多层视差背景模块

#### [BackgroundObject](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/BackgroundObject.h) (又称 `BackgroundLayer`)
* **核心职责**：代表单个独立的视差背景图层，保存它的材质信息、视差因子、平铺模式，并负责生成它本帧的世界空间精灵数据。
* **主要成员**：
  * 资源信息：`Image2D* image` 贴图指针。
  * 配置：`int renderOrder`（排序深度，用于映射 zIndex），`double parallaxFactor`（视差滑动比率），`double zoomFactor`（缩放响应比率），`BackgroundDrawMode drawMode`（平铺模式：单张、横向平铺、固定相机模式）。
  * 逻辑变换：`double centerX, centerY`（初始化设计的静态世界中心坐标），`double runtimeCenterX, runtimeCenterY`（被相机位移视差补偿修正后的当前帧绘制世界中心坐标），`double drawW, drawH` 渲染物理长宽。
* **核心接口**：
  * `updateRuntimeTransform(double parallaxOffsetX, double parallaxOffsetY)`：传入过滤后的相机有效移动参考量，应用公式：`runtimeCenterX = centerX + parallaxOffsetX * (1.0 - parallaxFactor)` 计算出本帧用于定位的世界坐标中心。
  * `collectSprites(RenderQueue& queue)`：根据 `drawMode` 生成精灵。在 `BACKGROUND_REPEAT_X` 下会以 `runtimeCenterX` 为核心向左右复制并生成多个精灵压入队列以覆盖整个相机视口。

#### [BackgroundManager](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/BackgroundManager.h)
* **核心职责**：多层背景图层集合管理器，负责更新所有图层由于相机位移产生的运行时位置偏移，并决定其渲染排序深度。
* **主要成员**：
  * `std::vector<BackgroundObject> layers`：背景图层容器列表。
* **核心接口**：
  * `updateRuntimeTransforms(double dx, double dy)`：在 `Level::update` 中捕获本帧相机发生的逻辑平移，遍历并驱动所有 `layers` 运算 `updateRuntimeTransform`。
  * `collectSprites(RenderQueue& queue)`：遍历各背景层并按 `renderOrder` 转换为精灵压入队列（负数 renderOrder 会渲染在瓦片和角色之后，正数则成为遮挡瓦片和玩家的前景层）。

---

### 7. 地图与瓦片系统

#### [TileMap](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/TileMap.h)
* **核心职责**：瓦片地图的宿主，负责解析文本地图格栅，根据配置自动判定各图块格子的物理碰撞特性（Solid，单向，None），并为相机可视区生成瓦片精灵数据。
* **主要成员**：
  * 尺寸定义：`int tileW, tileH` 贴图格子宽高，`int drawW, drawH` 实际绘制世界宽高，`int mapRows, mapCols` 格子行列数。
  * 地图网格：`int** grid` 存放格子的二维数组，`TileInstance** instances` 对应的瓦片实例二维数据，`TileCollisionType** collisions` 碰撞阻挡定义网格。
  * `Image2D* tileset` 瓦片拼图纹理指针。
* **核心接口**：
  * `loadFromText(const std::string& content)`：逐字符扫描地图定义文本，生成瓦片类型、物理碰撞模式（例如读取到土墙则判定为 `TILE_COLLISION_SOLID`，草台判定为 `TILE_COLLISION_TOP_HALF_ONE_WAY`），并填充 `TileInstance` 数组。
  * `collectSprites(RenderQueue& queue)`：读取当前 `gCamera` 的世界视口边界，换算出左上至右下的可视行列区间（粗裁剪剔除），仅为视口内的瓦片实例生成 `Sprite` 压入渲染队列。

#### [TileInstance](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/TileTypes.h) (以及 `TileTypes.h`)
* **核心职责**：单个瓦片实例的数据对象。由于每个瓦片可能包含独立的空间位移偏置或材质大小缩放，由此对象具体承载，为后续的瓦片动态特效或精细编辑打下基础。
* **主要成员**：
  * `int tileId` 对应 tileset 上的格号。
  * `int row, col` 所在网格行列。
  * `double centerX, centerY` 瓦片的世界几何中心坐标。
  * `double offsetX, offsetY`，`double scaleX, scaleY` 偏移与缩放。
  * `double drawW, drawH` 瓦片最终渲染的世界尺寸。

---

### 8. UI 与本地化文本模块

#### [UIElement](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/UI.h) (以及 `UIBox`, `UIAnchor`, `UIElementState`)
* **核心职责**：UI控件基础类，工作在纯屏幕空间坐标下。具备对齐定位（靠边、居中）能力，实现 UI 位置和尺寸向期望目标滑动的插值过渡。
* **主要成员**：
  * 物理状态：`double x, y`（当前屏幕坐标），`double w, h`（当前物理尺寸）。
  * 动画目标：`double targetX, targetY`，`double targetW, targetH` 渐变插值目标。
  * 对齐控制：`UIAnchor anchor`（左上、右上、居中等锚点方式），`int marginX, marginY` 边缘留白。
  * 状态机：`UIElementState state`（HIDDEN, SHOWING, VISIBLE, HIDING），`bool active` 逻辑更新激活，`bool visible` 渲染激活。
  * `double moveSpeed` 平滑插值系数。
* **核心接口**：
  * `init(...)`：设定 UI 规格与锚点对齐规则，并利用 `makeUIBoxByAnchor` 计算初版目标位置。
  * `update()`：平滑动画插值入口：使用 `MathUtils::smoothTo` 驱动当前 `x, y` 逼近 `targetX, targetY`，驱动 `w, h` 逼近 `targetW, targetH`，在达到误差范围内后校正状态并置入 `VISIBLE` 或 `HIDDEN`。

#### [DialogueBox](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/DialogueBox.h)
* **核心职责**：继承自 `UIElement` 的打字机对话框组件。处理大写像素文字自动换行排版、最终所需高度的预计算，实现对话框尺寸根据文本内容的动态插值拉伸与自动延时收回。
* **主要成员**：
  * 文字载荷：`std::string fullText` 目标完整大写文本，`std::string displayText` 当前帧已截取的打字文本。
  * 进度：`double textProgress` 字符裁剪推进进度计数器。
  * 字体：`Image2D* fontTexture` 白字体像素字集图。
  * 倒计时：`bool isAutoCloseEnabled`，`double autoCloseTimer`。
  * 配置项：`DialogueConfig config`。
* **核心接口**：
  * `startDialogue(const std::string& text, ...)`：装载全新文本并转为大写，初始化并重置打字计时器，将目标高度 `targetH` 先设为起步高度。
  * `updateDialogue()`：每帧被调用，打字未完时递增进度并更新 `displayText`，然后调用 `calculateRequiredHeight(displayText)` 实时模拟当前已打出文字的自动换行排版过程，计算当前字量需要排几行、加行距后的理想高度，动态调用 `this->setTargetSize(config.boxW, currentH)` 写入 `targetH`。
  * `advance()`：按键推进对话。若字未打完，则瞬间截取全部内容显示，并计算最终满版文本所需高度将其设为 `targetH` 从而触发平滑拉伸；若字已打完，则直接滑动收回隐藏。
  * `calculateRequiredHeight(...)`：文字换行高度预计算逻辑。模拟字符渲染排版：每个字符物理宽度为 `charWidth * scale`，累加 `charSpacing`。若加上一个字符后宽度超出了对话框边距，则将 `lines` 计数加 1 并重置起始 X。最后计算出理想排版高度。

#### [LocalizationManager](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/LocalizationManager.h)
* **核心职责**：本地化多语言词典。从 JSON 文件载入不同语言下的翻译键值对，提供统一的字符串获取接口。
* **主要成员**：
  * `std::unordered_map<std::string, std::string> strings`：加载进内存的翻译词典。
* **核心接口**：
  * `loadLanguage(const std::string& path)`：读取 JSON 键值对文本并写入哈希表中。
  * `getString(const std::string& key)`：根据 ID 返回翻译字符串，未找到时返回原 key。

#### [UIManager](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/UI.h)
* **核心职责**：管理当前界面中的所有 UI 元素，负责层级关系、父子树对齐刷新和统一 update 迭代。
* **主要成员**：
  * `std::vector<UIElement> elements`：UI 元素实例集合。
* **核心接口**：
  * `addElement(UIElement elem)`：将 UI 注册入管理器并返回其唯一数组索引。
  * `update()`：统一迭代更新每个活跃 UI 的滑入滑出或动态宽高缩放缓动进度。

---

### 9. 时间、工具与底层封装

#### [TimerManager](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/TimerManager.h)
* **核心职责**：基于游戏帧更新驱动的通用倒计时计时器管理器。用于处理延时发生的逻辑，如踩中旗子后延迟动态生成奖励道具。
* **主要成员**：
  * `std::unordered_map<std::string, double> timers`：计时器键名与剩余帧数倒计时的映射关系。
* **核心接口**：
  * `setTimer(const std::string& name, double durationFrames)`：登记或重置一个指定倒计时帧数的计时器。
  * `update()`：遍历内存中所有的 `timers` 并对剩余帧数累减 1.0。
  * `isFinished(const std::string& name)`：轮询特定计时器，若其余额小于等于 0 且存在，则返回 true 并销毁该计时器。

#### [Image2D](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/Image2D.h)
* **核心职责**：智能贴图资源包装类，保存图片的像素宽高规格并持有 EasyX 底层 IMAGE 结构，避免了裸指针生命周期错乱。
* **主要成员**：
  * `IMAGE img` EasyX 真实图片存储，`int width, height` 图片的物理像素尺寸。

#### [ResourceManager](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/Resource.h)
* **核心职责**：集中缓存加载中心，利用哈希表保存已加载的图片和文本，防范因在循环中重复读写物理磁盘导致的性能卡顿。
* **主要成员**：
  * `std::unordered_map<std::string, Image2D*> images` 贴图资源库，`std::unordered_map<std::string, std::string> textContents` 文本配置库。

#### [Camera](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/Camera.h)
* **核心职责**：逻辑相机视口状态定义。以屏幕中心为首要锚点，管理场景的世界坐标、屏幕坐标、缩放与世界边界限制。
* **主要成员**：
  * `centerX, centerY` 当前视口中心世界坐标，`zoom` 全局图像放大比例。
  * `targetCenterX, targetCenterY` 目标跟随点。
  * `dx, dy` 相机本帧相对上帧发生的水平与垂直世界坐标位移量。
* **核心接口**：
  * `worldToScreenX() / worldToScreenY()`：变换公式：`screenX = halfWidth + (worldX - centerX) * zoom`。
  * `limitInWorld()`：结合最新的 `zoom` 算得当前逻辑可视宽高的半高 `halfW, halfH`，使用 `clamp` 钳制 `centerX / centerY` 保证相机不能滑出地图边界。

#### [CameraFollow](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/CameraFollow.h)
* **核心职责**：相机跟随插值追踪逻辑，根据目标坐标与鼠标在屏幕中的偏移，计算平滑的跟随目的地。
* **核心接口**：
  * `followSmooth()`：结合 Lerp 对目标位置进行追随，调用相机的坐标钳制方法，并计算相机的水平与垂直位移差值。

#### [Controller](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/Controller.h) 与 `PlayerController`
* **核心职责**：动作输入意图解耦器。负责将键盘硬件输入翻译为游戏逻辑通用的控制意图 `BehaviorIntent`，使实体物理更新不直接依赖键盘读取。
* **核心接口**：
  * `makeIntent(InputManager& input, bool isGod)`：根据按键（左右、Shift冲刺、Space跳跃、E交互等），生成标准意图载荷并返回。

#### [InputManager](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/Input.h)
* **核心职责**：EasyX 输入状态监听器，管理按键的单次按下、按下状态和鼠标屏幕物理坐标。
* **核心接口**：
  * `update()`：调用 Win32 的 `GetKeyState` 等 API，刷新这一帧的按键映射状态与鼠标屏幕坐标。

#### [LevelDebugger](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/LevelDebugger.h)
* **核心职责**：独立的调试数据搜集与交互诊断工具，隔离调试渲染和常规关卡调度。
* **主要成员**：
  * `DebugPanelData` 实时帧率及实体的最新坐标、速度结构体。
* **核心接口**：
  * `handleInput(...)`：响应 `F5-F11`，控制是否渲染碰撞包围盒或隐藏 Debug UI 面板。
  * `updateDebugLogs(...)`：监听实体落地 `onGround`、撞墙 `blockedByWorld` 等布尔值变化，打印带精确帧时间戳的终端日志。

#### [MathUtils](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/MathUtils.h)
* **核心职责**：提供纯数学算法。包含 `clamp` 区间限定、`lerp` 线性插值，以及 `smoothTo(current, target, speed)` 差值衰减阻尼器（公式为 `current + (target - current) * speed`）。

#### [GraphicsUtils](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/GraphicsUtils.h)
* **核心职责**：提供 EasyX 图像绘制的辅助处理，主要是透明通道的 `transparentimage` 和带有 alpha 混合的自定义绘图扩展方法。

---
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

## 游戏初始化与生命周期数据流

为了让你能够深刻理解底层代码的运作顺序，下面按照调用堆栈和帧循环机制，对“初始化阶段”与“某一帧 Tick 逻辑更新”的数据流动进行拆解。

---

### 1. 游戏进入主循环前发生了什么？（初始化流程）

在主游戏窗口启动并开始逻辑循环之前，程序会经历如下底层资源的载入和各个独立系统的挂载。

```text
main() [game.cpp]
  │
  ├── 1. 调用 initgraph() 初始化 EasyX 窗口，注册 loadUIFont() 从 Mojangles.ttf 加载像素字体。
  │
  ├── 2. 实例化全局输入管理器 InputManager 与关卡控制器 Level level。
  │
  ├── 3. 绑定关卡事件函数指针（如 level1_InitEvent、level1_UpdateEvent）到 Level。
  │
  └── 4. 调用 level.init() 启动关卡装载流水线：
           │
           ├── 4.1. initResources()：
           │        ResourceManager::loadLevelResources() 登记图片与地图文本物理路径。
           │        AnimationClipManager 加载并解析 animations.json 的动画片段并分配贴图指针。
           │
           ├── 4.2. loadTemplates()：
           │        EntityManager 载入 entity_templates.json 的实体属性规格配置，缓存入模板库。
           │
           ├── 4.3. initMap()：
           │        TileMap 设定格栅物理大小（16x16 像素，绘制尺寸 48x48 像素）。
           │        读取并解析 map.txt，生成瓦片实例 TileInstance 网格，并映射 solid 与一向平台碰撞。
           │        计算并获取最终关卡世界逻辑像素宽度与高度。
           │
           ├── 4.4. loadEntities()：
           │        EntityManager 读取 entities.json 的初始实体（包括受控玩家、旗杆、金币等）。
           │        在 200 个槽位的固定连续对象池 entities 中完成初始化，挂载动画组件与包围盒。
           │
           ├── 4.5. levelDebugger.init(uiManager)：
           │        初始化调试控制台的各面板分区 UI。
           │
           ├── 4.6. collisionManager.clearHistory()：
           │        重置并排空上一关残留的实体对重叠检测历史集合 lastOverlapPairs。
           │
           ├── 4.7. 锁定受控角色并 setControlTarget(Id)：
           │        寻找带有 controlled 属性的实体，把键盘输入控制的目标绑定到该实体 ID 上。
           │
           ├── 4.8. initBackground()：
           │        打开 backgrounds.json，解析多层背景配置，创建 BackgroundObject 并登记 renderOrder。
           │        根据相机初始状态设定 centerX/centerY，最后将图层加入背景管理器中。
           │
           ├── 4.9. 像素对话框与本地化文字加载：
           │        LocalizationManager 装载 localization.json 翻译词典。
           │        DialogueBox 绑定像素字体贴图，初始化底部对齐中心锚点，默认隐藏并放至屏幕外。
           │
           └── 4.10. 执行 level1_InitEvent 回调：
                    遍历活跃实体，定位 CHECKPOINT 类型的旗杆实体，微调其碰撞盒偏移量。
```

---

### 2. 游戏循环中的某一 Tick 发生了什么？（典型 Tick 场景）

在进入 `while(true)` 循环后，游戏以每秒约 60 帧的速度不断 Tick。下面通过两个高频的经典案例，详细剖析本帧中各系统的具体数据流动和执行流：

#### 场景 A：玩家触发旗帜 ➡️ 延时生成金币并弹出 Dialog（复合事件行为）

当玩家运动包围盒在某帧与旗杆发生重叠，到生成香蕉奖励并弹出对话框的系列 Tick 级演进如下：

##### 【Tick 1：触旗碰撞检测与状态自治】
1. **重叠扫描 (`CollisionManager::updateOverlapEvents`)**：
   * 在 `Level::update` 阶段，调用 `collisionManager.updateOverlapEvents` 遍历 `EntityManager::getActiveIndices`。
   * 检测到 **[Player 实体 ID: 0]** 与 **[Checkpoint 实体 ID: 1]** 的 AABB 包围盒（由各自的 `CollisionBox` 结合 `x, y` 坐标算得）发生重叠相交。
   * 双方实体的 `overlapping` 状态位被置为 true，同时将对方实体的信息写入各自的 `currentOverlaps` 向量容器中。
   * 碰撞去重与日志：生成配对唯一键 `"0_1"` 并与上一帧 `lastOverlapPairs` 缓存比对。发现是新发生的重叠，控制台打印调试日志，并将键登记入 `lastOverlapPairs`。
2. **重叠反馈分发 (`CollisionManager::resolveEntityOverlaps`)**：
   * 调用实体的自治函数 `resolveOverlaps()`。
   * **[Checkpoint 实体 ID: 1]** 执行自身逻辑：由于在 `currentOverlaps` 中找到了 `PLAYER` 类型，表明被玩家触碰。
   * 触发升旗动作：将其 `Animator` 的当前播放状态切换为升旗动画 `"flag_out"`，推进其动画状态，并将逻辑事件标记 `flagActivatedJustNow` 设为 `true`。

##### 【Tick 2：事件触发与计时器注册】
1. **关卡事件监听与 Timer 注册 (`LevelEvents.cpp::level1_UpdateEvent`)**：
   * 在本 Tick 的逻辑处理阶段，执行绑定的关卡更新回调。
   * 回调函数遍历活跃实体，发现 **[Checkpoint 实体 ID: 1]** 的 `flagActivatedJustNow` 值为 `true`。
   * 脚本立即消费该信号（重置为 `false`），并调用管理器：`timerManager.setTimer("checkpoint_spawn_banana", 30.0)`。这会在 `TimerManager` 内部哈希表注册一个剩余 30 帧（约 0.5 秒）的倒计时计时器。

##### 【Tick 3 至 Tick 32：计时器推进】
1. **计时器自减与动画推进**：
   * 在每一帧的 `Level::update` 中，执行 `timerManager.update()`，将名为 `"checkpoint_spawn_banana"` 计时器的剩余帧数递减 1.0。
   * 此时，旗杆实体的 `AnimationPlayer` 正常在每帧递增帧计时器并推进 `"flag_out"` 动作，旗子动画一帧帧往上升。

##### 【Tick 33：计时器结束，触发香蕉生成与 UI 弹窗】
1. **Timer 完成响应**：
   * `LevelEvents.cpp::level1_UpdateEvent` 轮询检测 `timerManager.isFinished("checkpoint_spawn_banana")` 返回 `true`。
2. **动态香蕉延迟生成请求**：
   * 脚本读取旗杆实体当前的坐标 X，在其上方偏移 64 像素计算出香蕉生成位置。
   * 调用 `entityManager.queueSpawnEntity(spawnX, spawnY, "Banana")`。这不会立即往活跃数组插入，而是将新实体的生成请求（包括坐标、模板名等）缓存入 `spawnQueue` 中。
3. **加载本地化多语言文本**：
   * 调用 `localizationManager.getString("checkpoint_hit")`，根据当前语言，查得翻译文本为 `"CHECKPOINT REACHED. PROGRESS SAVED."`。
4. **对话框装载与滑入动画激活 (`DialogueBox`)**：
   * 调用 `dialogueBox.startDialogue(text, config)`：将文本全部转为大写，清空打字进度，并设定目标高度为初始高度（98像素）。
   * 调用 `dialogueBox.showDialogueWithOffset(0, 400)`：刷新对话框的对齐目标（屏幕底部），计算在完全隐藏时应该下移 400 像素作为起始坐标，把当前的绘制坐标 `x, y` 强行设在下边缘外，并使 state 转移为 `UI_SHOWING`。
5. **打字机动态排版与高度平滑拉伸**：
   * `dialogueBox.updateDialogue()`：
     * 打字进度 `textProgress` 每一帧按 `readSpeed` 递增，根据当前进度对 `fullText` 截取子字符串赋值给 `displayText`。
     * 调用 `calculateRequiredHeight(displayText)` 实时模拟当前已打出文字的自动换行排版过程，计算当前字量需要排几行、加行距后的理想高度，动态调用 `this->setTargetSize(config.boxW, currentH)` 写入 `targetH`。
   * `dialogueBox.update()`：
     * 自动执行：`y = MathUtils::smoothTo(y, targetY, moveSpeed)`，`h = MathUtils::smoothTo(h, targetH, moveSpeed)`。
     * 对话框在打字机逐字蹦出的过程中，不仅从屏幕下方平滑往上滑，而且高度也随着排版行数的变多平滑拉伸。
6. **帧末实体落地 (`EntityManager::processSpawns`)**：
   * 逻辑末端执行 `entityManager.processSpawns`。系统将从 `deadIndices` 中获取空闲槽位索引（假设为槽位号 5），清空其“上辈子”的残留状态，调用 `Entity::reset` 装载 `"Banana"` 模板属性，将其状态与首帧剪辑赋予完毕，最后把索引 5 登记进 `activeIndices`。香蕉正式动态降临关卡。
7. **精灵渲染与稳定排序**：
   * `Level::draw()`：
     * 清空 `RenderQueue`。
     * 收集所有演员的 Sprite 并设定 `zIndex`：背景图层 `zIndex` 为负数，瓦片 `zIndex` 为 0，主角和香蕉 `zIndex` 为 0，UI 和调试框 `zIndex` 设为最大。
     * 排序：`renderQueue.sort()` 采用稳定排序，香蕉和玩家同样是 Z-Index = 0，但它们由于被依次提交，会根据提交的物理顺序安全排在背景后方绘制。
     * 画面输出：渲染器顺序循环 `drawSprite` 呈现在 EasyX 双缓冲区。在最上层绘制 `dialogueBox`，玩家看到对话框一边上滑一边变长，打字机白字在对话框内跳跃。

---

#### 场景 B：键盘输入向右狂奔 ➡️ 物理位移阻挡限制 ➡️ 角色动画状态机实时切换

玩家从原地站立，按下向右方向键和 Shift 狂奔的这一帧 Tick 中，底层的数据流动过程如下：

##### 1. 输入接收与翻译 (`InputManager` & `PlayerController`)
* 全局最外层循环中，`input.update()` 捕获到当前键盘按键：**[右方向键]** 和 **[Left Shift 键]** 处于压下状态。
* 实体 ID 0 标识为 `controlled = true`，进入受控分支。
* `playerController.makeIntent` 将捕获状态翻译为统一行为意图：
  ```cpp
  BehaviorIntent (moveX = 1.0, wantSprint = true, wantJump = false)
  ```

##### 2. 运动学解算与 Speculative AABB 碰撞检测 (`MovementHandle` & `CollisionManager`)
* **MovementHandle** 获取该意图：
  * 检测到 `wantSprint = true` 且在地面上，将角色的最大移动速度倍增为狂奔速度。
  * 根据水平加速度，计算出期望在本帧移动的水平偏移量 `wantMoveX = 7.5`。
* **CollisionManager 水平探测**：
  * `MovementHandle` 内部调用 `collisionManager.getAllowedMoveX(owner, wantMoveX, tiles, activeEntities)`。
  * `CollisionManager` 获取实体的 CollisionBox 局部偏移，并叠加实体 X 坐标，生成世界空间包围盒 `RectBox`。
  * 将该包围盒沿 X 轴向右拉伸延伸 7.5 像素，遍历周围 solid 瓦片。如果检测到右方 4 像素处有 solid 墙体格子：
    * 碰撞检测逻辑阻断多余位移，将原本 7.5 像素的位移截断，修正并返回仅允许移动的极限安全距离 `allowedDx = 4.0`。
  * 实体的世界中心 X 坐标被安全设定为 `x = x + 4.0`，同时更新 `syncRenderSpriteWorldDrawData()` 精灵世界坐标。

##### 3. 动画状态机状态判断与切换规则匹配 (`Animator`)
* `Level::updateEntities` 调用 `entities[idx].updateAnimator(intent)`。
* 实体持有的 **Animator** 执行状态检测：
  * 从宿主实体处读取最新运行参量：`vx = 4.0`（有向右速度），`sprinting = true`（正在狂奔），`onGround = true`（在地面上），朝向 `currentFacingDirection = FACING_RIGHT`。
  * 遍历 transitionRules 状态机转换表，发现当前处于 `"idle"` 或 `"walk"` 的实体状态符合如下过渡规则：
    * 条件：`velocityX > 0` 且 `sprinting == true` 且 `onGround == true`。
    * 目标状态：`"run_r"`。
  * 状态机决策将当前状态改为 `"run_r"`。对比映射表得知 `"run_r"` 对应的动画素材片段名为 `"player1_run_r"`。
  * 状态机向实体的播放器组件发出切歌命令：`owner.getAnimation().setClip(myClips["player1_run_r"])`。
  * `AnimationPlayer` 载入狂奔动画片段，重置当前帧为第 0 帧，重新开启序列帧时钟。

##### 4. 动画序列帧推进与纹理区域写入 (`AnimationPlayer`)
* `Level::updateEntities` 调用 `entities[idx].updateAnimatedSprite()`。
* **AnimationPlayer** 驱动时间推进：
  * 内部的 `frameTimer` 累加 1。当其累加值到达该动画片段指定的播放间隔 `frameInterval`（如 6 帧）时，`frameTimer` 归零，`currentFrame` 计数加 1。
  * 将当前帧写入精灵：`writeCurrentFrameTo(renderSprite)`。
  * 换算图像坐标：根据 `currentFrame` 值，利用狂奔动画剪辑的参数，计算当前应裁剪序列图的第几行第几列：
    ```cpp
    int srcX = sourceStartX + (currentFrame % columns) * (frameWidth + spacingX);
    int srcY = sourceStartY + (currentFrame / columns) * (frameHeight + spacingY);
    ```
  * 将贴图资源指针 `imageSource` 及该裁剪矩形（`srcX, srcY, frameWidth, frameHeight`）写入实体的 `renderSprite` 精灵实例中。
* **同步世界绘制中心**：
  * 调用 `syncRenderSpriteWorldDrawData()`。结合实体最新的 X 坐标与 `renderSprite` 的偏移量，算出该精灵在世界中的中心坐标，完成一帧内的数据封闭。

##### 5. 渲染提交与双缓冲输出
* 关卡绘制阶段，`renderQueue.submit(entity.getSprite())` 将角色的狂奔帧精灵提交到渲染队列。
* 稳定排序后，Renderer 接收到该精灵数据，最终执行 EasyX 的 `putimage` 进行内存双缓冲级绘制。
* 玩家在屏幕上看到角色在墙角顺滑切换到了狂奔的奔跑动作姿态，并被墙体阻挡无法穿墙。

---
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
