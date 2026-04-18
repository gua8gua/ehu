# Ehu ECS 关系与调用说明

本文档说明 Ehu 当前 ECS 的三部分内容：

- 模块之间的关系（`Entity` / `World` / `Components` / `Scene` / `SceneLayer` / `Application`）
- 代码实现要点（数据结构与关键 API）
- 主线程中每帧如何调用 ECS（更新 + 渲染提交流程）

---

## 1. ECS 关系总览

在当前实现里，ECS 不是独立跑在“系统调度器”中，而是由场景与渲染层按帧驱动：

1. `Application` 维护激活的 `Scene` 集合，并驱动所有 `Layer::OnUpdate`。
2. `Scene` 持有一个 `World`，实体与组件都存放在 `World` 中。
3. `SceneLayer` 在更新阶段调用 `Scene::OnUpdate`，在渲染提交阶段从 `World` 查询组件并写入 `RenderQueue`。
4. `TransformComponent` + `CameraComponent` 会在提交前做一次同步（`RunCameraSync`）。

简化关系如下：

```text
Application (主循环)
  -> Activated Scenes (Scene*)
       -> World
            -> Entity (id + generation)
            -> Component pools (Transform/Sprite/Mesh/Camera/Tag/Id)

SceneLayer
  -> OnUpdate: scene->OnUpdate(ts)
  -> SubmitTo: RunCameraSync(world) + Each<...>(...) 提取可渲染实体
```

---

## 2. 关键对象职责

### 2.1 Entity：轻量句柄

- 定义在 `Ehu/src/Ehu/ECS/Entity.h`。
- 结构为 `{ id, generation }`。
- `id` 可复用，`generation` 在销毁后自增，用于防止“悬空句柄误用”。

### 2.2 World：实体生命周期 + 组件存储 + 查询迭代

- 定义在 `Ehu/src/Ehu/ECS/World.h` 与 `World.cpp`。
- 核心能力：
  - `CreateEntity()` / `DestroyEntity()` / `IsValid()`
  - `AddComponent<T>()` / `RemoveComponent<T>()`
  - `GetComponent<T>()` / `HasComponent<T>()`
  - `Each<C...>(func)` 进行组件组合迭代
- 存储模型：
  - 每种组件类型对应一个 `ComponentPool<T>`（本质为 `unordered_map<Entity, T>`）
  - `m_Pools` 以 `ComponentTypeId -> IComponentPool` 管理所有组件池
- 实体回收：
  - 销毁时移除全部组件
  - `generation++`
  - `id` 放入 `m_FreeList`，后续可复用

### 2.3 Components：纯数据

定义在 `Ehu/src/Ehu/ECS/Components.h`，当前主要组件：

- `TransformComponent`：位置/旋转/缩放，脏标记 + 延迟计算世界矩阵
- `SpriteComponent`：2D 提交参数（尺寸、颜色、排序、透明）
- `MeshComponent`：3D 提交参数（VAO、索引数、颜色、排序、透明）
- `CameraComponent`：`Camera*`（非拥有）
- `TagComponent`：名称（仅编辑器/调试标识）
- `RenderFilterComponent`：渲染通道（`RenderChannel`，如 `Default/UI/Debug`）
- `PhysicsFilterComponent`：碰撞层与掩码（`CollisionLayer` + `CollisionMask`）
- `IdComponent`：`UUID`

### 2.4 Scene：ECS 的宿主

- 定义在 `Ehu/src/Ehu/Scene/Scene.h` 与 `Scene.cpp`。
- `Scene` 内部持有 `World m_World`。
- `CreateEntity()` 默认添加 `IdComponent` 与 `TagComponent`。
- `SetMainCamera(Entity)` 指定主相机实体；`GetMainCamera()` 从 `CameraComponent` 取指针。

### 2.5 SceneLayer：ECS 到渲染队列的桥接

- 定义在 `Ehu/src/Ehu/Renderer/SceneLayer.h` 与 `SceneLayer.cpp`。
- 在 `SubmitTo` 中：
  1. 对每个激活场景执行 `RunCameraSync(world)`（Transform -> Camera）
  2. 用 `Each<Transform, Sprite>` / `Each<Transform, Mesh>` 提取可渲染实体
  3. 按 `RenderFilterComponent.Channel` 与 `SceneLayer.RenderChannel` 匹配后提交到 `RenderQueue`

---

## 3. 主线程调用时序（每帧）

主线程入口在 `Application::Run()`（`Ehu/src/Ehu/Core/Application.cpp`）。

每帧与 ECS 相关的关键顺序：

1. 遍历层栈，调用 `layer->OnUpdate(m_TimeStep)`
  - 若该层是 `SceneLayer`，会进一步对每个激活场景调用 `scene->OnUpdate(ts)`。
2. 清空 `RenderQueue`，再次遍历层栈
  - 对实现了 `IDrawable` 的层调用 `SubmitTo(*m_RenderQueue)`。
3. `SceneLayer::SubmitTo` 内部执行 ECS 查询与提取
  - 先 `RunCameraSync(world)`
  - 再 `world.Each<...>` 按组件组合提交 Sprite/Mesh
4. `RenderQueue->Sort()` 后 `FlushAll()` 完成实际绘制。

可以理解为：**OnUpdate 阶段改数据，SubmitTo 阶段读 ECS 并生成渲染命令**。

---

## 4. 代码示例：在主线程驱动下使用 ECS

下面示例演示“创建实体 + 每帧更新 + 自动参与渲染提交”的最小链路：

```cpp
// 1) 初始化场景并注册到 Application（通常在你的 Layer/应用初始化阶段）
Ehu::Ref<Ehu::Scene> scene = Ehu::CreateRef<Ehu::Scene>();
Ehu::Application::Get().RegisterScene(scene, true);

// 2) 创建实体并添加组件
Ehu::Entity e = scene->CreateEntity();
auto& world = scene->GetWorld();
world.AddComponent(e, Ehu::TransformComponent{});
world.AddComponent(e, Ehu::SpriteComponent{});

if (auto* tag = world.GetComponent<Ehu::TagComponent>(e))
    tag->Name = "Player";
world.AddComponent(e, Ehu::RenderFilterComponent{ Ehu::RenderChannel::Default });

// 3) 每帧逻辑更新（在 OnUpdate / OnUpdateScene 中）
world.Each<Ehu::TransformComponent>([](Ehu::Entity, Ehu::TransformComponent& t) {
    t.SetPosition(t.Position.x + 0.01f, t.Position.y, t.Position.z);
});
```

只要该实体拥有正确组件，且 `RenderFilterComponent.Channel` 与当前 `SceneLayer` 的通道一致，就会在该层的 `SubmitTo` 中被提取并渲染。

---

## 5. 常见使用约定（当前项目）

- 实体创建/销毁优先走 `Scene::CreateEntity()` / `Scene::DestroyEntity()`，不要绕开 Scene 直接管理生命周期。
- 访问组件统一走 `World::GetComponent<T>()` / `HasComponent<T>()`，空指针/缺组件都视为合法状态。
- 不在 UI 层直接改渲染对象，优先通过 ECS 组件修改状态，由 `SceneLayer` 统一提取。
- `CameraComponent` 里的 `Camera*` 为非拥有指针，生命周期由 `Scene` 的相机容器负责。

这样可以保证“数据在 ECS、流程在主线程、渲染在提交阶段”三者分离清晰。

---

## 6. `World` 本质上管理些什么？

从当前代码实现看，`World` 更接近“实体句柄管理器 + 组件数据仓库”。

### 6.1 `World` 内部到底在存什么结构？

- **实体集合**：`m_LiveEntities` 保存当前仍然有效的实体句柄列表（用于快速拿到所有实体）。
- **实体合法性信息**：`m_Generations` 与 `m_FreeList`
  - `m_Generations[id]` 记录这一槽位的 generation，用于判断一个 `Entity{id, generation}` 是否仍有效
  - `m_FreeList` 记录可复用的 `id`，当 `DestroyEntity` 时回收
- **组件数据**：`m_Pools`
  - `m_Pools` 是 `ComponentTypeId -> IComponentPool` 的映射
  - 每种组件类型对应一个 `ComponentPool<T>`，内部使用“实体稠密数组 + 组件稠密数组 + 实体到索引映射”
  - 因此组件查询与遍历的基础结构是：**“稠密存储 + O(1) 索引定位”**

`World` 管理的是“`Entity` 的有效性 + 若干类型组件的 per-type 稠密池 + 供查询迭代的实体列表”。

### 6.2 基础操作是通过操作什么实现的？

对应到 `World.h` / `World.cpp`，基础操作主要分两类：

1. **实体生命周期**
  - `CreateEntity()`：从 `m_FreeList` 取回可复用 `id`，或使用 `m_NextId` 分配新 `id`；同时把实体加入 `m_LiveEntities`
  - `DestroyEntity(Entity)`：遍历所有 `m_Pools`，对每个组件池调用 `Remove(e)`；然后 `generation++` 并把 `id` 推回 `m_FreeList`，同时从 `m_LiveEntities` 移除
2. **组件访问与查询**
  - `AddComponent<T>(Entity, T&&)` / `RemoveComponent<T>(Entity)`：定位到对应 `ComponentPool<T>`（通过 `GetTypeId<T>()` 找到 `m_Pools` 中的池），再在稠密池中增删
  - `GetComponent<T>(Entity)` / `HasComponent<T>(Entity)`：同样先定位池，再通过索引映射查找组件
  - `Each<Components...>(func)`：
    - 先选择组件最少的池作为 pivot，减少候选遍历量
    - 对候选 `Entity e`，再分别 `GetComponent<Other>(e)` 检查其它组件是否都存在
    - 都存在才调用 `func(e, componentRefs...)`

因此，“基础操作”本质都围绕 `m_LiveEntities / m_Generations / m_FreeList / m_Pools (dense pools)` 这些结构展开。

---

## 7. 脚本组件与实体交互（当前实现）

当前 ECS 已包含脚本组件：

- `ScriptComponent`（定义于 `ECS/Components.h`）
  - `ClassName`：脚本类全名（如 `Game.PlayerController`）
  - `Fields`：脚本字段快照（用于 Inspector 编辑、序列化与热重载迁移）

### 7.1 谁驱动脚本执行

- `Application::Run()` 驱动各层 `OnUpdate`。
- `SceneLayer::OnUpdate` 会调用 `Scene::OnUpdate`。
- `Scene::OnUpdate` 在 Play 模式下调用 `ScriptEngine::OnSceneUpdate(scene, dt)`。

这意味着脚本系统并不是独立线程或独立调度器，而是被主线程帧循环驱动。

### 7.2 实体与脚本如何交互

- 触发条件：实体拥有 `ScriptComponent` 且至少一个 `Instances[n].ClassName` 非空。
- 运行流程（`ScriptEngine::OnSceneUpdate`）：
  1. 通过 `ClassName` 找到/缓存托管类（方法与字段元信息）
  2. 通过 `ScriptRegistry` 按 `Scene + Entity + InstanceId` 维度创建或复用 `ScriptRuntimeInstance`（`GCHandle` 保活）
  3. 将 `ScriptComponent.Instances[n].Fields` 写入托管对象字段
  4. 首帧触发 `OnCreate`，每帧触发 `OnUpdate`
  5. 将托管对象字段回写到 `ScriptComponent.Instances[n].Fields`

因此“脚本改实体状态”的常见路径是：

1) C# 代码调用 InternalCall 进入 C++（当前已接入日志与 deltaTime，组件类 API 后续扩展），或  
2) 通过脚本字段回写影响组件系统读取到的数据。

### 7.3 脚本实例由谁管理

- 脚本实例不存放在 `World`，由 `ScriptRegistry` 管理（键为 `Scene + Entity + InstanceId`），`ScriptEngine` 负责按实例列表顺序调度。
- 当场景销毁时，`Scene::~Scene` 调用 `ScriptEngine::OnSceneDestroyed(scene)`，释放对应 `GCHandle`。
- 当实体销毁时，`Scene::DestroyEntity` 会调用 `ScriptEngine::OnEntityDestroyed(scene, entity)` 回收实例。
- 当项目切换或停用场景时，`Application::DeactivateAllScenes()` 会卸载 App 程序集，脚本实例与类缓存被清理。

这个边界保证了 `World` 仍是纯 ECS 数据层，不直接持有 Mono 对象。