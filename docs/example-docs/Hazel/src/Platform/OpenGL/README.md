# OpenGL 平台实现

基于 OpenGL 的渲染管线实现，对应 Renderer 模块中的抽象接口。

## 功能概览

| 组件 | 功能 |
|------|------|
| **OpenGLContext** | GraphicsContext 实现：创建 OpenGL 上下文、SwapBuffers |
| **OpenGLBuffer** | 顶点/索引 Buffer 的 OpenGL（VBO/IBO）实现 |
| **OpenGLVertexArray** | VertexArray 的 OpenGL VAO 实现 |
| **OpenGLShader** | Shader 的编译、链接与 uniform 设置 |
| **OpenGLTexture** | 2D 纹理的创建、绑定与参数 |
| **OpenGLFramebuffer** | Framebuffer 的 FBO 实现，支持渲染目标与深度/模板 |
| **OpenGLRendererAPI** | RendererAPI 实现：清屏、绘制、状态设置等 |
| **OpenGLUniformBuffer** | UniformBuffer 的 UBO 实现 |

## 架构要点

- 所有渲染命令经 Renderer/RenderCommand 下发，最终由 OpenGLRendererAPI 与各 OpenGL* 对象执行。
- 当前引擎以 OpenGL 为主要渲染后端，Vulkan 相关依赖（Dependencies.lua）为后续或工具链预留。
