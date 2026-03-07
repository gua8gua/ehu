# Hazel 引擎模块 (Hazel)

与平台无关的引擎功能模块集合，按职责分为 Core、Events、Renderer、Scene、Project、Scripting、ImGui、Math、Physics、Debug、Utils、UI。

## 子模块


| 模块                                 | 说明                                                                                                             |
| ---------------------------------- | -------------------------------------------------------------------------------------------------------------- |
| [Core](./Core/README.md)           | 应用、Layer/LayerStack、日志、窗口、输入、键鼠码、时间步、定时器、UUID、文件系统、断言、Buffer、入口点、平台检测                                          |
| [Events](./Events/README.md)       | 事件基类、应用事件、键盘事件、鼠标事件                                                                                            |
| [Renderer](./Renderer/README.md)   | 渲染器、Renderer2D、渲染命令、RendererAPI、Buffer、Shader、Texture、Framebuffer、VertexArray、UniformBuffer、相机、字体、图形上下文、MSDF 等 |
| [Scene](./Scene/README.md)         | 场景、实体、场景相机、场景序列化、组件、ScriptableEntity                                                                           |
| [Project](./Project/README.md)     | 项目、项目配置、项目序列化                                                                                                  |
| [Scripting](./Scripting/README.md) | 脚本引擎（Mono/C#）、ScriptGlue（C# 与引擎桥接）                                                                             |
| [ImGui](./ImGui/README.md)         | ImGui 层、ImGui 构建集成                                                                                             |
| [Math](./Math/README.md)           | 数学工具                                                                                                           |
| [Physics](./Physics/README.md)     | 2D 物理（Physics2D）                                                                                               |
| [Debug](./Debug/README.md)         | 性能/调试工具（Instrumentor）                                                                                          |
| [Utils](./Utils/README.md)         | 平台工具（PlatformUtils）                                                                                            |
| [UI](./UI/README.md)               | UI 相关头文件                                                                                                       |


## 依赖关系

- Core 提供应用框架、窗口、输入、时间步等基础能力。
- Events 被 Application 与各 Layer 使用。
- Renderer 依赖 Core（如 Buffer）与 Platform 的图形 API 实现。
- Scene 使用 entt 作为 ECS 底层，并依赖 Renderer、Physics、Scripting。
- Project 与 Scene 序列化配合，供编辑器与运行时加载项目/场景。

