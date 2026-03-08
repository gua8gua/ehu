# Scene 模块

场景与实体系统：基于 ECS（entt）的场景管理、实体、组件与序列化。

## 功能概览


| 组件                   | 功能                                                                                         |
| -------------------- | ------------------------------------------------------------------------------------------ |
| **Scene**            | 场景：创建/销毁实体、运行时/模拟/编辑器更新、视口大小变更、实体查找（按名/UUID）、复制实体、主相机获取；内部使用 entt::registry                |
| **Entity**           | 实体句柄：对 Scene 中某个 entt 实体的引用，提供组件添加/获取/删除与父子层级                                              |
| **SceneCamera**      | 场景内相机组件用的相机（正交/透视、宽高比等）                                                                    |
| **SceneSerializer**  | 场景序列化/反序列化（如 YAML 格式），与 Project 资源路径配合                                                     |
| **Components**       | 组件定义：Tag、Transform、SpriteRenderer、CircleRenderer、Camera、Script、Rigidbody2D、BoxCollider2D 等 |
| **ScriptableEntity** | 可脚本化实体基类：C# 脚本通过继承此类获得 OnCreate/OnDestroy/OnUpdate 等生命周期                                   |


## 架构要点

- Scene 持有 `entt::registry`，Entity 为对 registry 中 entity 的封装。
- 运行时/模拟时由 Scene 的 OnUpdateRuntime/OnUpdateSimulation 驱动物理与脚本更新；编辑器下为 OnUpdateEditor。
- 序列化由 SceneSerializer 与 Project 的资产路径配合，实现场景的保存与加载。

