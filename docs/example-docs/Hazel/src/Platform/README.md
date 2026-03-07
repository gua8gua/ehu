# Platform 平台实现

引擎在各平台与图形 API 上的具体实现，与 `Hazel/` 下的抽象接口一一对应。

## 子模块

| 模块 | 说明 |
|------|------|
| [Windows](./Windows/README.md) | Windows 平台：窗口、输入、平台工具 |
| [OpenGL](./OpenGL/README.md) | OpenGL 渲染 API：上下文、缓冲、着色器、纹理、帧缓冲、渲染 API、UniformBuffer 等 |

## 架构要点

- Core 的 Window、Input、PlatformUtils 由 Windows 实现。
- Renderer 的 Buffer、VertexArray、Shader、Texture、Framebuffer、RendererAPI、UniformBuffer 等由 OpenGL 实现。
- 通过预编译或运行时选择可扩展为多平台（如 Linux/Mac）与多图形 API（如 Vulkan）。
