# EasyX 2D 游戏框架：草图设计机制与 C++ 代码映射讲解

本文件夹存放了我自己在项目重构与演进过程中绘制的**核心设计草图（Sketches）**。为了方便我以后回顾、理解数据流动、职责边界以及底层的具体实现，本篇文档将重点结合我当初画这些草图时的思路，详细整理当前 C++ 代码的底层运行机制。

---

## 目录
1. [静态对象池与双索引哈希寻址（对应：静态对象池与map索引的对应关系？.png）](#1-静态对象池与双索引哈希寻址对应静态对象池与map索引的对应关系png)
2. [数据驱动的动画状态机链路（对应：涉及动画链路的小想法以及外部json配置的可行性.png）](#2-数据驱动的动画状态机链路对应涉及动画链路的小想法以及外部json配置的可行性png)
3. [相机坐标转换与背景对象视差自治系统（对应：视差背景.png）](#3-相机坐标转换与背景对象视差自治系统对应视差背景png)
4. [解耦精灵与 Z-Sorting 稳定排序渲染流（对应：渲染层级关系的想法.png）](#4-解耦精灵与-z-sorting-稳定排序渲染流对应渲染层级关系的想法png)
5. [屏幕空间 UI 布局、动态尺寸拉伸与过渡（对应：mind.png 与 UI 模块）](#5-屏幕空间-ui-布局动态尺寸拉伸与过渡对应mindpng-与-ui-模块)

---

## 1. 静态对象池与双索引哈希寻址
* **关联草图**：[静态对象池与map索引的对应关系？.png](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/draft/静态对象池与map索引的对应关系？.png)

### 📌 草图核心想法
在游戏运行时，频繁地通过 `new` / `delete` 动态分配和释放实体，会导致严重的**内存碎片化**和主循环卡顿。同时，如果在遍历活跃实体数组的过程中直接动态增加或删除元素，会造成**迭代器失效**导致程序崩溃。
我当初的想法是：预分配一个连续大数组作为对象池，维护活跃与空闲索引，并通过哈希表建立实体唯一 ID 到数组下标的常数时间映射。

### 💻 C++ 代码映射与实现
我把这个机制写在了 [EntityManager.h/.cpp](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/EntityManager.h) 中：

#### ① 对象池预分配与双索引列表
* **连续对象池**：`std::vector<Entity> entities`。在构造时我直接预分配 `200` 个槽位：
  ```cpp
  entities.resize(200);
  ```
* **双索引划分**：
  * `std::vector<size_t> activeIndices`：仅保存当前在关卡中存活并需要参与物理更新/渲染的实体在池中的下标。
  * `std::vector<size_t> deadIndices`：保存当前空闲、可被复用的池下标。初始化时，0~199 的所有下标均登记在 `deadIndices` 中。
* **物理更新时的极速遍历**：更新逻辑时，我只遍历 `activeIndices`，完全跳过空闲槽位，执行效率极高：
  ```cpp
  for (size_t idx : activeIndices) {
      entities[idx].update(...);
  }
  ```

#### ② ID 到 Slot 下标的常数时间哈希映射
每个实体在诞生时会被分配一个全局唯一的 `EntityID`。为了在 $O(1)$ 时间内定位实体而不需要全表遍历，我使用了 `std::unordered_map`：
* **结构定义**：
  ```cpp
  std::unordered_map<EntityID, size_t> idToIndex;
  ```
* **工作流**：外部系统拿着 ID 获取实体指针时，先通过哈希表查到池下标，再直接以数组下标寻址：
  ```cpp
  Entity* EntityManager::getEntity(EntityID id) {
      auto it = idToIndex.find(id);
      if (it != idToIndex.end()) {
          return &entities[it->second];
      }
      return nullptr;
  }
  ```

#### ③ 帧末延迟安全生成 (`processSpawns`)
当玩家打碎宝箱生成金币时，绝对不能直接插入 `activeIndices`。我采用了**延迟生成缓存队列**：
1. **生成申请**：调用 `queueSpawnEntity()`，将生成位置和模板名塞入暂存队列 `spawnQueue`。
2. **帧末落地**：逻辑更新完毕后，调用 `processSpawns()`：
   * 从 `deadIndices` 尾部弹出一个空闲槽位下标 `slot`。
   * 调用 `entities[slot].reset(...)` 抹除该槽位上一辈子的数据残留，并装载新模板。
   * 更新哈希映射：`idToIndex[newId] = slot`。
   * 将 `slot` 追加到 `activeIndices`。

---

## 2. 数据驱动的动画状态机链路
* **关联草图**：[涉及动画链路的小想法以及外部json配置的可行性.png](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/draft/涉及动画链路的小想法以及外部json配置的可行性.png)

### 📌 草图核心想法
动作逻辑切换不应当在 C++ 代码中写死大量的 `if-else`。实体的物理状态（速度、是否在空中）应该与最终播放的画面帧裁剪范围解耦。
我当初设想：外部通过配置动画参数（如行数、列数、间隔），由状态机过渡条件驱动，将状态参数映射为具体的 Clip 片段，最后由播放器计算出源图纹理裁剪坐标写入 Sprite。

### 💻 C++ 代码映射与实现
我设计的这个动作链条涉及了 [AnimationClip](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/AnimationClip.h)、[AnimationPlayer](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/AnimationPlayer.h)（即草图中的播放器）与 [Animator](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/Animator.h)（状态机）：

```text
 ┌──────────────┐      根据物理参量(vx, onGround)      ┌───────────┐
 │ Behavior /   │ ─────────────────────────────────> │ Animator  │
 │ Physical Stat│                                    └─────┬─────┘
 └──────────────┘                                          │
                                                条件匹配成功,切歌
                                                           │
                                                           ▼
 ┌──────────────┐      计算行列及 srcX/Y 并写入        ┌───────────┐
 │ RenderSprite │ <───────────────────────────────── │Animation  │ (AnimationClip 缓存)
 └──────────────┘                                    │  Player   │
                                                     └───────────┘
```

#### ① 状态机条件转移 (`Animator::update`)
实体内部持有 `Animator`。每帧在 `Level::updateEntities` 中：
1. 状态机读取实体的运行参数：水平速度 `vx`，是否在空中 `InAir`，是否处于冲刺 `sprinting` 等。
2. 扫描从模板配置中解析出的 `TransitionRule` 数组。如果当前状态是 `"idle"`，且满足规则：
   * 条件：`velocityX > 0` 且 `onGround == true` 且 `sprinting == false`。
   * 触发转移：将当前状态 `currentState` 切换为 `"walk"`。
3. 从 `stateToClipName` 字典查出状态 `"walk"` 对应美术资源的片段名为 `"player1_walk_r"`，随即驱动播放器载入该 Clip：
   ```cpp
   owner.getAnimation().setClip(owner.getClipForState(nextState));
   ```

#### ② 播放器裁剪定位 (`AnimationPlayer::writeCurrentFrameTo`)
* **帧推进**：在每帧 Tick 中，播放器计数器 `frameTimer` 自增。达到当前片段的播放间隔时重置，并让 `currentFrame` 指向下一张序列帧。
* **数学换算裁剪坐标**：根据列数 `frameColumns` 与帧号算出行列，再结合间距换算出像素级裁剪起点：
  ```cpp
  int frameCol = currentFrame % frameColumns;
  int frameRow = currentFrame / frameColumns;
  int srcX = sourceStartX + frameCol * (frameWidth + frameSpacingX);
  int srcY = sourceStartY + frameRow * (frameHeight + frameSpacingY);
  ```
* **写回 Sprite**：调用 `targetSprite.setSource(texture, srcX, srcY, frameWidth, frameHeight)`，完成渲染精灵源图坐标的写入。

---

## 3. 相机坐标转换与背景对象视差自治系统
* **关联草图**：[视差背景.png](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/draft/视差背景.png)

### 📌 草图核心想法
2D 横板游戏中，摄像机（Camera）负责确定我们看世界的范围，但不能直接修改物体的逻辑坐标。
早期方案中，背景视差移动被生硬地写在 Camera 的映射或者统一的绘制公式中，导致背景与镜头逻辑极度耦合。
我当初的想法是：背景图层应该拥有自己的“视差系数（parallaxFactor）”和独立的位置变换逻辑。由背景层“自我更新”运行时坐标，最后再像普通世界物体一样丢给渲染器进行标准的相机坐标变换绘制。

### 💻 C++ 代码映射与实现
我把这个思想实现在了 [Camera.h](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/Camera.h)、[BackgroundObject.h/.cpp](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/BackgroundObject.h) 中：

#### ① 世界坐标到屏幕坐标转换（Camera 职责单一化）
Camera 不负责背景偏置，它只基于**视口中心点（centerX/Y）**和**缩放比率（zoom）**提供唯一的正向转换接口：
```cpp
double Camera::worldToScreenX(double worldX) const {
    return WINDOW_WIDTH / 2.0 + (worldX - centerX) * zoom;
}
```

#### ② 背景对象自治视差更新 (`BackgroundObject`)
每个背景层代表一个独立的逻辑对象，保存有：设计初始世界中心 `centerX`，视差系数 `parallaxFactor`（越远越小，如 0.1；前景较大，如 1.2），以及当前帧绘制位置 `runtimeCenterX`。
* **视差偏移计算**：
  在 `Level::update` 中，相机更新平移量并计算出背景专用的视差平移量 `dx`（已过滤由于贴边限制和 zoom 修正带来的多余平移噪音）。
* **背景运行时坐标的自我修正**：
  ```cpp
  void BackgroundObject::updateRuntimeTransform(double parallaxOffsetX, double parallaxOffsetY) {
      // 核心视差公式：
      runtimeCenterX = centerX + parallaxOffsetX * (1.0 - parallaxFactor);
      runtimeCenterY = centerY + parallaxOffsetY * (1.0 - parallaxFactor);
  }
  ```
  * 当 `parallaxFactor = 0.0` 时（天空），runtimeCenterX 完全跟随相机平移，使其在屏幕上保持相对静止。
  * 当 `parallaxFactor = 0.5` 时，runtimeCenterX 仅补偿相机平移量的一半，实现一半的滚动速度。
  * 当 `parallaxFactor = 1.2` 时（前景海洋），runtimeCenterX 反向补偿，实现超快滚动的深度感。

---

## 4. 解耦精灵与 Z-Sorting 稳定排序渲染流
* **关联草图**：[渲染层级关系的想法.png](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/draft/渲染层级关系的想法.png)

### 📌 草图核心想法
如果背景、地图瓦片、实体角色各自执行 EasyX 的 `putimage` 绘制，渲染的先后顺序将死死受限于代码的顺序，极难管理。
我当初的想法是：背景、瓦片和角色都不要自己画图。它们在 Draw 阶段只做一件事——生成一个轻量的 `Sprite`（精灵单帧数据）并提交（submit）到一个统一的渲染队列中。队列对精灵按照 `zIndex` 进行排序，最后由 Renderer 统一、顺序地画在双缓冲内存中。

### 💻 C++ 代码映射与实现
我写的这套精灵收集渲染流由 [RenderQueue.h/.cpp](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/RenderQueue.h) 承载：

```text
 ┌──────────────┐
 │ Background   │ ─┐
 └──────────────┘  │
 ┌──────────────┐  │ submit    ┌─────────────┐  std::stable_sort  ┌─────────────┐  drawAll  ┌──────────┐
 │ TileMap      │ ─┼─────────> │ RenderQueue │ ─────────────────> │Sorted Queue │ ────────> │ Renderer │
 └──────────────┘  │ (Sprite)  └─────────────┘                    └─────────────┘           └──────────┘
 ┌──────────────┐  │
 │ EntityPool   │ ─┘
 └──────────────┘
```

#### ① 精灵解耦收集 (`collectSprites`)
在每帧的 `Level::draw()` 开始时，清空队列：
```cpp
renderQueue.clear();
```
各子系统分别遍历并往队列里压入 Sprite 副本：
```cpp
backgroundManager.collectSprites(renderQueue); // 负数 zIndex
tileMap.collectSprites(renderQueue);           // 默认 zIndex = 0
entityManager.collectSprites(renderQueue);     // 默认 zIndex = 0
```

#### ② 稳定层级排序 (`std::stable_sort`)
收集完成后，调用 `renderQueue.sort()` 进行排序。
* **为什么使用稳定排序（`stable_sort`）？**
  如果使用快速排序等非稳定排序算法，当两个精灵的 `zIndex` 同样为 0 时（如处于同一地平线的玩家和香蕉），它们的相对位置在每次排序后可能会发生随机跳变，造成严重的图像闪烁或遮挡混乱。
  我使用了 `std::stable_sort`，这能保证当 `zIndex` 相同时，**严格保留它们被提交进队列的先后物理顺序**，从而确保渲染表现极其稳定：
  ```cpp
  void RenderQueue::sort() {
      std::stable_sort(items.begin(), items.end(), [](const sprite& a, const sprite& b) {
          return a.zIndex < b.zIndex;
      });
  }
  ```

#### ③ 统一渲染分发与视口裁剪
排序完成后，整个队列的精灵顺序是从后往前的（远景 ➡️ 近景 ➡️ 玩家 ➡️ 前景 ➡️ UI）。Renderer 统一遍历队列执行 EasyX 绘制。
在此过程中，我还通过 `gCamera` 的屏幕可见矩形与 `sprite` 的世界中心包围盒进行碰撞相交判断，非可视区域内的精灵直接不予绘制，从而节省了底层绘制带宽， Debug 面板中显示的真实渲染数就是该裁剪统计后的精确数据。

---

## 5. 屏幕空间 UI 布局、动态尺寸拉伸与过渡
* **关联草图**：[mind.png](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/draft/mind.png) (UI部分)

### 📌 草图核心想法
UI 元素（如血条、对话框）与世界空间的物体不同，它们应该钉在屏幕上的固定位置，不受 Camera 视口平移和 zoom 缩放的影响，直接工作在屏幕空间。
同时，UI 应该具备优雅的状态机（隐藏、滑入中、显示中、退出中）和物理缓动，支持动态内容形变。

### 💻 C++ 代码映射与实现
我写的 UI 框架在 [UI.h/.cpp](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/UI.h) 与像素打字机对话框 [DialogueBox.cpp](file:///c:/Users/lpy16/OneDrive/Desktop/c++/GraphAndObjTest/graphAndObjectTest/DialogueBox.cpp) 中得到了深度体现：

#### ① 屏幕锚点对齐 (`UIAnchor`)
在创建 `UIElement` 时，我不需要指定死坐标，而是指定 `UIAnchor`（如 `UI_BOTTOM_LEFT`、`UI_CENTER`）和偏置值 `marginX / Y`：
* 在初始化和屏幕大小调整时，自动调用 `makeUIBoxByAnchor`，根据窗口尺寸自动计算出对应的目标位置坐标 `targetX / targetY`。

#### ② 物理缓动与形变插值 (`MathUtils::smoothTo`)
UIElement 的真实位置 `x, y` 和宽高 `w, h` 并不会瞬间跳变，而是每帧在其 `update()` 中，向目标坐标和大小缓慢插值逼近，展现出极其流畅的视觉动感：
```cpp
void UIElement::update() {
    if (!active) return;
    
    // 阻尼插值衰减：
    x = MathUtils::smoothTo(x, targetX, moveSpeed);
    y = MathUtils::smoothTo(y, targetY, moveSpeed);
    w = MathUtils::smoothTo(w, targetW, moveSpeed);
    h = MathUtils::smoothTo(h, targetH, moveSpeed);
}
```

#### ③ 对话框文字排版换行与打字同步生长
为了在打字机逐字蹦出文字时，对话框能够“像长高一样往上顶起”，我在 `DialogueBox` 中实现了以下高度计算公式：
* **动态行数与换行预计算 (`calculateRequiredHeight`)**：
  在打字机未走完时，截取当前已经打完的部分字符串 `displayText` 塞入模拟排版算法中。程序模拟从 paddingLeft 开始横向排列每个像素字符，超过最大宽度限制（`boxW - paddingRight`）则折行，`lines` 自增 1。
* **最终拉伸高度数学模型**：
  $$neededHeight = paddingTop + paddingBottom + (lines 	imes charHeight 	imes scale) + (lines - 1) 	imes lineSpacing$$
* 每帧打出新字时，利用此高度公式计算当前文本所需的物理高度，并将其作为全新的 `targetH` 赋予 UI 元素：
  ```cpp
  double currentH = calculateRequiredHeight(displayText);
  this->setTargetSize(config.boxW, currentH);
  ```
  由于 `y` 坐标是对齐底部中心点的，当目标高度 `targetH` 变大时，其对齐后的目标 Y 坐标 `targetY` 会同步减少。于是，在 UI 的 `update()` 阶段，对话框不仅高度 `h` 变高，其位置 `y` 也会由于插值平滑往上滑，实现了**自适应高度拉伸的打字动态效果**。

---

## 💡 总结
这套代码架构的精髓在于我把逻辑层与数据表现层彻底解耦了。
这些草图是我当初的设计灵感图纸，而我最终写出来的 C++ 代码，通过连续内存对象池、解耦的精灵提交队列、高度自治的背景视差计算和阻尼 UI 缓动，实现了一个高效、规范、流畅的 2D 游戏底层框架。
