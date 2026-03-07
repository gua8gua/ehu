# Hazel.Scene 命名空间 (C#)

场景与实体在脚本侧的 API：Entity 引用、组件访问等。

## 主要类型

| 文件/类型 | 功能 |
|-----------|------|
| **Entity** | 实体句柄的 C# 表示，可获取/添加组件、访问子实体等；内部通过 UUID 或句柄与 C++ Entity 对应 |
| **Components** | 组件类型的 C# 封装（如 Transform、SpriteRenderer），供脚本读写实体上的组件数据 |

## 架构要点

- 脚本中通过 `Entity` 与 `Components` 访问当前实体及同场景其他实体，所有跨边界调用通过 InternalCalls 进入 C++ ScriptGlue。
