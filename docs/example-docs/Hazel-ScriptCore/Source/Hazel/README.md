# Hazel 命名空间 (C#)

脚本层根命名空间：数学类型、输入、内部调用声明。

## 主要类型

| 文件/类型 | 功能 |
|-----------|------|
| **Vector2/3/4** | 向量类型，与引擎侧数据传递一致 |
| **KeyCode** | 键码枚举，与 Core/KeyCodes 对应 |
| **Input** | 静态接口：键盘/鼠标查询，内部通过 InternalCalls 调用 C++ |
| **InternalCalls** | 声明由 ScriptGlue 注册的 C 函数入口，供 C# 调用引擎（Entity、Component、Input、Log 等） |

## 子命名空间

| 路径 | 说明 |
|------|------|
| [Scene](./Scene/README.md) | Entity、Components 等场景相关 C# 类型 |
