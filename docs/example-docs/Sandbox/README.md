# Sandbox 示例应用

用于演示与测试引擎用法的示例游戏/应用，链接 Hazel 静态库。

## 目录结构


| 路径                     | 说明                    |
| ---------------------- | --------------------- |
| [src](./src/README.md) | 入口、示例 Layer（2D 与通用示例） |
| assets                 | 示例资源：着色器、字体等          |
| premake5.lua           | Sandbox 项目定义          |
| imgui.ini              | ImGui 布局配置            |


## 功能概览

- **SandboxApp**：Application 子类，注册 ExampleLayer 或 Sandbox2D 等 Layer。
- **Sandbox2D**：2D 示例层：相机控制、四边形/圆形/纹理绘制、文本等，展示 Renderer2D 用法。
- **ExampleLayer**：通用示例层：基础渲染与 ImGui 演示。

## 架构要点

- 作为独立可执行程序运行，不依赖 Hazelnut；适合验证引擎 API 与渲染/脚本行为。

