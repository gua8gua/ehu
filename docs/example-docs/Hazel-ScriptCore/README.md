# Hazel-ScriptCore

C# 脚本层 API：供游戏脚本使用的类型与内部调用声明，与引擎 C++ 侧通过 Mono 与 ScriptGlue 桥接。

## 目录结构

| 路径 | 说明 |
|------|------|
| [Source](./Source/README.md) | C# 源码根，包含 Hazel 命名空间下的 API 与 Scene 子命名空间 |
| Properties | 程序集属性（premake 中声明，可为空） |
| premake5.lua | 项目定义，输出 DLL 到 `../Hazelnut/Resources/Scripts` |

## 功能概览

- **数学/输入**：Vector2/3/4、KeyCode、Input（封装引擎内部调用）。
- **场景**：Entity、Components（对应引擎侧组件的 C# 视图）。
- **InternalCalls**：声明由 C++ ScriptGlue 注册的内部调用，供 C# 调用引擎功能。

## 架构要点

- 编译产物被复制到 Hazelnut 的脚本目录，由编辑器与运行时通过 Mono 加载。
- 脚本中继承 ScriptableEntity 的类会由 ScriptEngine 反射并挂到实体上，OnCreate/OnUpdate/OnDestroy 由引擎在对应时机调用。
