# Renderer 模块

渲染抽象与 2D 渲染管线：API 无关的接口与 OpenGL 实现，以及相机、着色器、纹理、帧缓冲等。

## 功能概览


| 组件                               | 功能                                                                               |
| -------------------------------- | -------------------------------------------------------------------------------- |
| **Renderer**                     | 渲染器单例：Init/Shutdown、窗口大小变更、BeginScene/EndScene、Submit（Shader + VertexArray + 变换） |
| **Renderer2D**                   | 2D 批处理渲染：四边形、圆形、线段、文本（MSDF 字体）；统计信息                                              |
| **RenderCommand**                | 提交到渲染命令队列的封装（清屏、设置视口等）                                                           |
| **RendererAPI**                  | 渲染 API 抽象（当前为 OpenGL），封装底层绘制调用                                                   |
| **Buffer**                       | 顶点/索引缓冲区抽象；OpenGL 实现为 OpenGLVertexBuffer、OpenGLIndexBuffer                       |
| **VertexArray**                  | 顶点数组对象，绑定 VertexBuffer + IndexBuffer                                             |
| **Shader**                       | 着色器程序（顶点/片段，支持 GLSL）；OpenGL 实现支持源码与文件加载                                          |
| **Texture**                      | 纹理抽象；2D 纹理、子纹理等                                                                  |
| **Framebuffer**                  | 帧缓冲，用于渲染到纹理（如编辑器视口）                                                              |
| **UniformBuffer**                | 统一缓冲区，向着色器传递 uniform 块                                                           |
| **OrthographicCamera**           | 正交相机（视图+投影矩阵）                                                                    |
| **OrthographicCameraController** | 正交相机控制器（平移、旋转、缩放）                                                                |
| **EditorCamera**                 | 编辑器用相机（弧球等控制）                                                                    |
| **Camera**                       | 相机基类/通用接口                                                                        |
| **Font / MSDFData**              | 字体与 MSDF 图集数据，用于文本渲染                                                             |
| **GraphicsContext**              | 图形上下文抽象（OpenGL 上下文创建与交换）                                                         |


## 架构要点

- 上层只依赖 Renderer、Renderer2D、RenderCommand 等抽象；具体 API 由 Platform/OpenGL 实现。
- 2D 渲染流程：BeginScene 设置相机 → 多次 Draw 调用（四边形/圆形/线/文本）→ EndScene 提交批处理。

