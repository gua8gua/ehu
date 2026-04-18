# ECS / Scripting 重构基线（2026-03）

本文件用于冻结重构前后关键行为，作为回归校验清单。

## 1. 生命周期基线

- 实体创建统一通过 `Scene::CreateEntity()`，默认附加 `IdComponent` 与 `TagComponent`。
- 实体销毁路径：
  - `Scene::DestroyEntity()`
  - 脚本侧触发 `OnDestroy`（若脚本实现）
  - `World::DestroyEntity()` 移除全部组件并回收实体槽位
- 场景销毁路径：
  - `Scene::~Scene()`
  - 脚本侧对场景内脚本实体执行 `OnDestroy`
  - `ScriptRegistry::RemoveScene()`
- Play/Stop 切换：
  - `Application::SetPlayMode(false)` 会触发脚本实例 `OnDestroy` 并清理运行时实例
  - `Application::SetPlayMode(true)` 会重置 `OnCreate` 状态，保证再次进入 Play 时重新执行 `OnCreate`

## 2. ECS 存储与查询基线

- 组件池从哈希表改为稠密存储：
  - `entities[]`（实体列表）
  - `components[]`（组件列表）
  - `entity -> index` 映射
- `World::Each<...>()` 使用最小组件池作为 pivot，避免固定按第一个组件全量扫描。
- `const World::Each()` 只提供只读组件引用，不再 `const_cast`。

## 3. 渲染与物理筛选基线

- 渲染筛选从生命周期 Layer 解耦：
  - 使用 `RenderFilterComponent.Channel`
  - `SceneLayer` 通过自身 `RenderChannel` 提取匹配实体
- 物理筛选模型：
  - `PhysicsFilterComponent.CollisionLayer`
  - `PhysicsFilterComponent.CollisionMask`
  - 规则：`(a.Mask & b.Layer) != 0 && (b.Mask & a.Layer) != 0`
- 场景序列化：
  - 新增 `RenderChannel`、`PhysicsFilter`
  - 兼容读取旧 `RenderLayer` 键并映射到 `RenderChannel=Default`

## 4. 脚本运行时基线

- `Scene` 通过 `IScriptRuntime` 桥接脚本，避免硬依赖 `ScriptEngine` 静态调用。
- `ScriptEngine` 提供调用上下文（当前 `Scene` + `Entity`），供 InternalCall 安全访问 ECS 数据。
- 已接入 InternalCall：
  - `Log_Native`
  - `GetDeltaTime_Native`
  - `GetCurrentEntityId_Native`
  - `GetPosition_Native`
  - `SetPosition_Native`

- 多脚本实例基线：
  - `ScriptComponent` 为 `Instances[]` 列表，每个实例含 `InstanceId`、`ClassName`、`Fields`
  - 同一实体允许挂载多个脚本实例（含同类型重复）
  - 生命周期执行顺序按 `Instances[]` 顺序
  - `ScriptRegistry` 映射键为 `Scene + Entity + InstanceId`

## 5. 序列化与编辑器写回基线

- 场景文件中 `Tag` 改为带引号格式，支持空格名称。
- Inspector 对脚本字段写回通过 `ScriptComponent.Instances[n].Fields`（按 `InstanceId` 定位），与运行时字段同步保持兼容。
- `DuplicateEntity` 通过 `ComponentReflectionRegistry` 的克隆描述表复制组件，减少手工硬编码复制点。
