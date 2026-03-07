# Sandbox 源码 (src)

示例应用的入口与各示例 Layer。

## 主要文件

| 文件 | 说明 |
|------|------|
| **SandboxApp.cpp** | 入口：创建 Application，按需 PushLayer（如 Sandbox2D、ExampleLayer） |
| **Sandbox2D.h/cpp** | 2D 示例层：正交相机控制器、四边形/圆形/线/文本绘制、纹理与 MSDF 字体示例 |
| **ExampleLayer.h/cpp** | 通用示例层：简单几何与 ImGui 演示 |

## 架构要点

- 每个 Layer 在 OnAttach 中初始化所需 Shader/Texture 等，在 OnUpdate 中驱动逻辑与相机，在 OnImGuiRender 中绘制调试 UI。
