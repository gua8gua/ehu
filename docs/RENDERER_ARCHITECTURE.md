# Ehu 渲染架构调度关系

本文档描述渲染系统的分层、调度顺序与数据流，便于理解各模块职责与调用关系。

---

## 一、分层概览

渲染架构分为两层，职责边界如下：

```
┌─────────────────────────────────────────────────────────────────┐
│  Renderer 层（平台无关）                                            │
│  Camera、Renderer2D、Scene、Material、Sorting、Culling、PostFX 等   │
│  不依赖 OpenGL/DirectX/Vulkan 等具体 API                            │
└─────────────────────────────────────────────────────────────────┘
                                    │
                                    │ 提交绘制命令 / 设置 uniform
                                    ▼
┌─────────────────────────────────────────────────────────────────┐
│  Render API 层（API/平台相关）                                       │
│  RendererAPI、VertexBuffer、IndexBuffer、Shader、Texture、States 等   │
│  由 Backends/OpenGL_GLFW 等实现                                      │
└─────────────────────────────────────────────────────────────────┘
```

---

## 二、主循环调度顺序

每帧由 `Application::Run()` 驱动，调度顺序如下：

```
Application::Run() 每帧
│
├─ 1. RendererAPI::Get().SetClearColor(...)
├─ 2. RendererAPI::Get().Clear()                    ← Render API 层：清屏
│
├─ 3. for (Layer* layer : m_LayerStack)
│       layer->OnUpdate()                           ← 各 Layer 更新逻辑
│       └─ 用户 Layer 内可调用：
│          Renderer2D::BeginScene(camera)
│          Renderer2D::DrawQuad(...)                ← Renderer 层：提交绘制
│          Renderer2D::EndScene()
│
├─ 4. m_ImGuiLayer->Begin()
├─ 5. for (Layer* layer : m_LayerStack)
│       layer->OnImGuiRender()                     ← ImGui 调试 UI
├─ 6. m_ImGuiLayer->End()
│
└─ 7. m_Window->OnUpdate()                         ← 事件轮询、交换缓冲
```

**要点**：

- 清屏由 **Application** 直接调用 **RendererAPI**，不经过 Renderer 层。
- 实际绘制由 **Layer::OnUpdate()** 内调用 **Renderer2D** 等完成。
- ImGui 在场景绘制之后、窗口刷新之前执行。

---

## 三、Renderer2D 场景调度

`Renderer2D` 采用 **BeginScene → Submit → EndScene** 模式：

```
Layer::OnUpdate()
│
├─ Renderer2D::BeginScene(OrthographicCamera)
│   └─ 写入 s_SceneData->ViewProjectionMatrix
│   └─ （待实现）绑定 Shader、上传 u_ViewProjection
│
├─ Renderer2D::DrawQuad(position, size, color)      ← 可多次调用
│   └─ （待实现）将四边形几何加入批处理队列
│
├─ Renderer2D::DrawQuad(...)                        ← 继续提交
│
└─ Renderer2D::EndScene()
    └─ （待实现）批量提交、flush 到 Render API
```

**数据流**：

1. **Camera** 提供 `GetViewProjectionMatrix()`，供 `BeginScene` 写入 `SceneData`。
2. **SceneData** 持有当前场景的 ViewProjection 等全局 uniform，供后续 `DrawQuad` 使用。
3. **DrawQuad** 提交的几何在 `EndScene` 时统一提交到 Render API 层。

---

## 四、初始化顺序

渲染相关组件的初始化顺序（在 `Application` 构造中）：

```
1. Window::Create()           ← 创建窗口、OpenGL 上下文
2. Input::Init()
3. RendererAPI::Init()         ← 创建 OpenGLRendererAPI 等
4. ImGuiLayer 创建并 PushLayer
5. （用户）Renderer2D::Init()  ← 应在首次绘制前、窗口创建后调用
```

**说明**：`Renderer2D::Init()` 由用户 Layer 在 `OnAttach` 中调用，或由 Application 在构造末尾调用。必须在 `RendererAPI::Init()` 之后、首次 `BeginScene` 之前执行。

---

## 五、模块职责与调用关系

| 模块 | 职责 | 被谁调用 | 调用谁 |
|------|------|----------|--------|
| **Application** | 主循环、清屏、层栈调度 | EntryPoint | RendererAPI、LayerStack、Window |
| **Layer** | 每帧更新、提交绘制 | Application | Renderer2D、Camera、Scene |
| **Renderer2D** | 2D 场景提交、批处理 | Layer | Camera、SceneData；（待）Shader、VertexArray、RendererAPI |
| **Camera** | 提供 View/Projection 矩阵 | Layer、Renderer2D | glm |
| **Scene / SceneNode** | 场景图、变换层次 | Layer | — |
| **Material** | 材质属性、Bind/Unbind | Layer、Renderer2D | （待）Shader |
| **RendererAPI** | 清屏、DrawIndexed 等 | Application、Renderer2D | Backends（OpenGL 等） |

---

## 六、未来扩展的调度关系

### 6.1 Render passes

多 pass 渲染时，可扩展为：

```
Application::Run()
├─ RendererAPI::Clear()
├─ for (Layer : m_LayerStack) layer->OnUpdate()
│   ├─ RenderPass::Begin("Opaque")
│   │   ├─ Renderer2D::BeginScene(camera)
│   │   ├─ DrawQuad(...)
│   │   └─ Renderer2D::EndScene()
│   └─ RenderPass::Begin("Transparent")
│       └─ ...
├─ PostFX::Apply()              ← 后期处理
└─ m_Window->OnUpdate()
```

### 6.2 Scene 与 Renderer 的衔接

当 Scene 接入渲染后：

```
Layer::OnUpdate()
├─ Renderer2D::BeginScene(camera)
├─ for (SceneNode* node : scene->GetRenderables())
│   ├─ Material::Bind()
│   ├─ Renderer2D::DrawQuad(node->GetWorldTransform(), ...)
│   └─ Material::Unbind()
└─ Renderer2D::EndScene()
```

### 6.3 Sorting / Culling

在 `EndScene` 内部或 Layer 提交前：

1. **Culling**：按相机视锥体剔除不可见物体。
2. **Sorting**：按材质、深度、透明度等排序，减少状态切换。
3. **Batch**：合并相同材质的绘制调用。

---

## 七、依赖方向（禁止反向依赖）

```
Application (Core)
    │
    ├──► RendererAPI (Platform)     Application 可调用 RendererAPI
    │         ▲
    │         │ 实现
    │    Backends/OpenGL_GLFW
    │
    └──► Layer
             │
             └──► Renderer2D (Renderer)    Layer 可调用 Renderer2D
                    │
                    ├──► Camera
                    ├──► Scene / Material
                    └──► （待）RendererAPI::DrawIndexed 等
```

**原则**：Renderer 层不包含 Platform/Backends 头文件；通过 RendererAPI 等抽象接口与底层通信。

---

## 八、对外接口（Renderer 模块）

### 8.1 统一头文件

```cpp
#include "Renderer/RendererModule.h"
```

包含：`Renderer`、`Camera`、`Renderer2D`、`Renderer3D`、`Scene`、`Material`。

### 8.2 初始化与关闭

- **Renderer::Init()**：由 `Application` 在 `RendererAPI::Init()` 之后调用（已接入 `Application` 构造）。
- **Renderer::Shutdown()**：由 `Application` 析构调用。

### 8.3 2D 绘制流程（Layer 内）

```cpp
void OnUpdate() override {
    Ehu::Renderer2D::BeginScene(m_Camera);
    Ehu::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.8f, 0.2f, 0.3f, 1.0f });
    Ehu::Renderer2D::EndScene();
}
```

### 8.4 按需包含

若只需部分接口，可单独包含（子目录已整理为 Camera/、Scene/、Material/）：

- `Renderer/Renderer.h` — Renderer::Init / Shutdown
- `Renderer/Camera/Camera.h` — OrthographicCamera、PerspectiveCamera
- `Renderer/Renderer2D.h` — Renderer2D
- `Renderer/Renderer3D.h` — Renderer3D（占位）
- `Renderer/Scene/Scene.h` — Scene、SceneNode
- `Renderer/Material/Material.h` — Material、MaterialProperties

---

## 九、文档索引

| 文件 | 内容 |
|------|------|
| [ARCHITECTURE.md](ARCHITECTURE.md) | 引擎整体模块依赖与分层 |
| [TODO.md](TODO.md) | 渲染相关待办（Render API 与 Renderer 层） |
| [README.md](README.md) | 项目说明与构建 |

---

*Renderer 层调度与 Render API（VertexBuffer、Shader、VertexArray、DrawIndexed）已实现并接入 Application 与 SandBox 示例。*
