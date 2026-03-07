# Scripting 模块

C# 脚本运行时与引擎桥接：基于 Mono 的脚本加载、实体/组件暴露、内部调用注册。

## 功能概览

| 组件 | 功能 |
|------|------|
| **ScriptEngine** | 脚本引擎：初始化/关闭 Mono、加载程序集、脚本类/方法缓存、实体实例管理、脚本字段（ScriptField/ScriptFieldInstance）与类型映射 |
| **ScriptGlue** | C# 与引擎的桥接：向 Mono 注册内部调用（Entity、组件、Input、日志等），使 C# 脚本可调用引擎接口 |

## 架构要点

- 脚本程序集来自 Hazel-ScriptCore 编译产物及用户脚本，由 ScriptEngine 加载并反射得到 ScriptableEntity 子类。
- ScriptGlue 在引擎初始化时注册 C# 可调用的 C++ 函数，实现 Entity、Transform、Input、KeyCode 等在 C# 侧的可用性。
- 脚本字段通过 ScriptFieldType 与 MonoClassField 映射，支持在编辑器中显示与序列化。
