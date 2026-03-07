# Physics 模块

2D 物理集成：与 Box2D 的封装与场景联动。

## 功能概览

| 组件 | 功能 |
|------|------|
| **Physics2D** | 2D 物理世界与刚体/碰撞体的创建与步进；与 Scene 中的 Rigidbody2D、BoxCollider2D 等组件配合 |

## 架构要点

- Scene 在运行时创建/持有 `b2World`，根据场景中的物理组件同步到 Box2D 并每帧 Step。
- 物理组件通过 SceneSerializer 序列化，便于在编辑器中编辑 2D 物理参数。
