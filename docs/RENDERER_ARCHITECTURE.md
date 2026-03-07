# Ehu 渲染架构调度关系

本文档描述渲染系统的分层、调度顺序与数据流，便于理解各模块职责与调用关系。

---

## 一、分层概览

渲染架构分为两层，职责边界如下：

```
┌─────────────────────────────────────────────────────────────────┐
│  Renderer 层（平台无关）                                            │
│  Camera、Renderer2D、SceneLayer、Material、RenderQueue、Sorting 等   │
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

## 二、三阶段流程与主循环

### 2.1 三阶段概览

- **第一阶段：逻辑 Tick（Logic Tick）**  
  `App::Run()` → `Layer::OnUpdate(TimeStep)`。Layer 作为驱动者，根据生命周期决定是否驱动场景；**Scene::OnUpdate(TimeStep)** 执行 ECS/Systems（物理、脚本等），仅更新实体状态，**不进行任何渲染调用**。逻辑与渲染解耦。

- **第二阶段：提取与提交（Extract & Submit）**  
  从“逻辑世界”到“渲染世界”的桥梁。Layer 通过 **Application::GetActivatedScenes()** 取得已激活场景，对归属本层的实体（**entity->GetRenderLayer() == this**）做视图裁剪（可扩展视锥剔除）、数据打包（Transform/Material/Mesh → DrawCall 包），提交到 **RenderQueue**。

- **第三阶段：渲染调度与执行（Dispatch）**  
  **Sort**：不透明物体按 LayerIndex → MaterialKey（Shader/Material 分组）→ 深度 Front-to-Back；透明物体按 LayerIndex → 深度 Back-to-Front。**Execution**：通过 RendererAPI（VertexArray、DrawIndexed 等）将队列中的指令发送给 GPU。

### 2.2 主循环顺序

每帧由 `Application::Run()` 驱动：

```
Application::Run() 每帧
│
├─ 1. RendererAPI::SetViewport / BeginRenderPass / SetClearColor / Clear()
│
├─ 2. 【Phase1】for (Layer* layer : m_LayerStack) layer->OnUpdate(TimeStep)
│       └─ SceneLayer 内：for (Scene* s : GetActivatedScenes()) s->OnUpdate(timestep)；再 OnUpdateScene(timestep)
│
├─ 3. m_RenderQueue->Clear()
├─ 4. 【Phase2】for (Layer* layer : m_LayerStack)
│       if (IDrawable* d = ...) m_RenderQueue->SetCurrentLayerIndex(i); d->SubmitTo(*m_RenderQueue)
│       └─ SceneLayer::SubmitTo 从所有已激活场景中提取 entity->GetRenderLayer()==this 的实体入队
│
├─ 5. 【Phase3】m_RenderQueue->Sort()  （不透明 Layer→Material→深度；透明 Layer→深度 Back-to-Front）
│
├─ 6. cam = 第一个 IProvidesCamera 的 layer->GetCamera()
├─ 7. if (cam) { SetDepthTest/SetCullFace；若有 2D 命令则 BeginScene→Flush2D→EndScene；
│               若有 3D 命令且透视相机则 BeginScene→Flush3D→EndScene }
│
├─ 8. m_ImGuiLayer->Begin() → 各 Layer OnImGuiRender() → End()
└─ 9. m_Window->OnUpdate()
```

**要点**：

- **Scene 不归 Layer 所有**：场景由 **Application::RegisterScene** 注册，Layer 通过 **GetActivatedScenes()** 访问；每个可渲染实体有 **SetRenderLayer(Layer*)**，仅当 `entity->GetRenderLayer() == 当前 Layer` 时被该层提交。
- **相机**由实现 **IProvidesCamera** 的 Layer 提供；清屏与 Flush 由 Application 统一执行。

---

## 三、2D 与 3D 渲染器：同一底层上下文与混用

**Renderer2D 和 Renderer3D 最终都作用于同一渲染底层上下文**：二者都通过 **RendererAPI::Get()**（即当前 Backend 的 OpenGL 实现）执行 `DrawIndexed`、状态设置等，绘制目标都是当前绑定的帧缓冲（默认为主窗口的后缓冲）。因此：

- **可以混用**：同一帧内先调用 `Renderer3D::BeginScene` / `Submit` / `EndScene`，再调用 `Renderer2D::BeginScene` / `DrawQuad` / `EndScene`，或反之；两次绘制会叠加到同一画面，**绘制顺序决定谁在上层**（后绘制的覆盖先绘制的，由深度测试与深度缓冲决定遮挡）。
- **状态共享**：若在 Layer 中设置了 `SetDepthTest(true)`、`SetCullFace(true)` 等，会同时影响后续的 2D 与 3D 绘制，直到再次修改或切换 RenderPass。按需在各自 BeginScene 前设置状态即可。
- **总结**：2D 与 3D 渲染器是同一 Render API 层上的两种提交方式，共用同一上下文，可自由混用；几何数据由调用方（如 Sandbox）或 Renderer 内部（如 2D 的四边形）提供，3D 渲染器只接收 VAO + 变换 + 颜色等“要画什么”的数据。
- **默认着色器**：默认 2D/3D 着色器的 **GLSL 源码由底层实现（Backend）持有**（如 `OpenGLShader.cpp`）；Renderer 层通过 `Shader::CreateDefault2D()` / `CreateDefault3D()` 获取 Shader 指针，仅做 Bind、SetMat4、SetFloat4 等设置与提交，不持有或修改着色器源码。需要改动默认着色器时，在对应 Backend 中修改。

---

## 四、场景模块与 SceneLayer（场景注册、实体归属层）

**场景模块（Scene/）** 与渲染模块分离，**仅作数据提供者**：不含 SubmitTo、不实现 IDrawable。场景由 **Application** 统一注册与激活；提交由 **SceneLayer** 从已激活场景中按 **实体所属层** 提取并入队。

### 场景注册与激活

- **Application::RegisterScene(Scene*, bool activated)**：Application 取得 Scene 所有权；析构或 **UnregisterScene** 时释放。
- **Application::GetActivatedScenes()**：返回当前已激活的场景列表，供 Layer 在 Phase1（驱动 Scene::OnUpdate）与 Phase2（Extract）使用。
- **SetSceneActivated(Scene*, bool)**：切换场景是否参与逻辑与提取。

### 实体归属层（RenderLayer）

- **SceneEntity::SetRenderLayer(Layer*)** / **GetRenderLayer()**：每个可渲染实体属于某一 Layer。Phase2 中，每个 Layer 只从所有已激活场景里提交 **entity->GetRenderLayer() == this** 的实体，实现“每层检查所有已激活场景中是否包含当前实体”的流程。

### 逻辑流程（推荐）

1. **创建 Layer 与 Scene**：`Layer* layer = new SceneLayer(camera)`（SceneLayer **不**再持有单个 Scene）；`Scene* scene = new MyScene(layer)`（或传入 layer 以便实体 SetRenderLayer(layer)）。
2. **填充场景**：创建 SceneEntity、设置 Mesh/Sprite 组件、**e->SetRenderLayer(layer)**、`scene->AddEntity(e)`。场景逻辑（如旋转）可在 **Scene::OnUpdate(TimeStep)** 中实现（子类重写）。
3. **注册与入栈**：`Application::Get().RegisterScene(scene, true)`；`PushLayer(layer)`。
4. **Phase1**：SceneLayer::OnUpdate 内对 `GetActivatedScenes()` 逐场景调用 **scene->OnUpdate(timestep)**，再调用 **OnUpdateScene(timestep)**（子类可选重写）。
5. **Phase2**：SceneLayer::SubmitTo 对每个已激活场景遍历实体，仅当 `e->GetRenderLayer() == this` 时打包提交到 RenderQueue。

### 场景与实体、组件

- **Scene**：管理 SceneEntity 列表；**virtual void OnUpdate(const TimeStep&)** 默认空，子类或系统在此执行 ECS/逻辑，仅更新状态不渲染。
- **SceneEntity**：由所在场景管理；**SetRenderLayer(Layer*)** 指定归属层；SetPosition/SetRotation/SetScale、MeshComponent、SpriteComponent 等。
- **MeshComponent** / **SpriteComponent**：同上（VAO、Color、SortKey、Transparent 等）。

### SceneLayer（引擎提供）

- **SceneLayer(Camera* camera)**：仅持有 Camera 所有权；**不**持有 Scene。构造后由用户注册 Scene、在实体上设置 RenderLayer 指向本层。
- **OnUpdate**：对 `Application::Get().GetActivatedScenes()` 逐场景调用 **scene->OnUpdate(timestep)**，再 **OnUpdateScene(timestep)**。
- **SubmitTo**：对每个已激活场景，将 **GetRenderLayer() == this** 的实体提取并 **SubmitMesh/SubmitQuad** 入队。

---

## 五、Renderer2D / RenderQueue 与 Flush

**Renderer2D** 仍为 **BeginScene → 提交 → EndScene**，但“提交”由 **Application** 通过 **RenderQueue::Flush2D()** 统一完成：

```
Application 取相机后
│
├─ Renderer2D::BeginScene(*cam)
├─ m_RenderQueue->Flush2D()     ← 按排序结果依次 DrawQuad，透明段自动 SetBlend
├─ Renderer2D::EndScene()
├─ 若为透视相机：Renderer3D::BeginScene(*p) → Flush3D() → EndScene()
```

**数据流**：

1. **Camera** 由 IProvidesCamera 提供，供 Application 传入 BeginScene。
2. **RenderQueue** 在每帧由各 IDrawable 的 SubmitTo 填充，Sort 后 Flush2D/Flush3D 按序提交到 Renderer2D/Renderer3D。
3. 各 Layer 的 OnUpdate 只更新逻辑；绘制由 SubmitTo → 队列 → Sort → Flush 统一完成。

---

## 六、初始化顺序

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

## 七、模块职责与调用关系

| 模块 | 职责 | 被谁调用 | 调用谁 |
|------|------|----------|--------|
| **Application** | 主循环、场景注册/激活、清屏、收集 SubmitTo、Sort、Flush | EntryPoint | RendererAPI、LayerStack、RenderQueue、Scene、Window |
| **Layer** | Phase1 OnUpdate 逻辑；Phase2 SubmitTo 从激活场景提取归属本层实体入队 | Application | Application::GetActivatedScenes、Camera |
| **Scene** | 管理 SceneEntity；OnUpdate 执行 ECS/逻辑（仅状态，不渲染） | SceneLayer::OnUpdate、Application（注册） | SceneEntity |
| **SceneEntity** | 变换与 Mesh/Sprite 组件；GetRenderLayer() 决定由哪一层提交 | Scene、SceneLayer::SubmitTo | - |
| **RenderQueue** | 收集命令；Sort（Layer→Material→深度/透明 Back-to-Front）；Flush2D/Flush3D | Application、IDrawable | Renderer2D、Renderer3D |
| **RendererAPI** | 清屏、DrawIndexed、状态 | Application、Renderer2D/3D | Backends（OpenGL 等） |

---

## 八、未来扩展（可选）

- **Render passes**：若需多 pass，可在 Application 内对 RenderQueue 分 pass 或扩展 Layer 提交接口。
- **Culling**：在 Sort 前按相机视锥体剔除；**Sorting** 已由 RenderQueue::Sort（不透明/透明）完成；**Batch** 可合并同材质绘制。

---

## 九、灰屏排查要点（流程梳理）

若窗口仅显示清屏色（灰/青）而无几何体，按下列顺序排查：

1. **OpenGL 上下文与深度缓冲**
   - 必须请求 **深度缓冲**（`GLFW_DEPTH_BITS`），否则 `Clear(ClearDepth)` 与深度测试无效。
   - 建议同时请求 **OpenGL 3.3**（`GLFW_CONTEXT_VERSION_MAJOR/MINOR`），使用 `GLFW_OPENGL_ANY_PROFILE` 避免 Core Profile 在某些环境导致创建失败；这样可保证有可用深度缓冲。

2. **层栈与相机**
   - 取相机时使用 **第一个** `IProvidesCamera` 的 Layer 的 `GetCamera()`。若 Example3D 在 ImGui 之后入栈，则不会被当作主相机。
   - 当前约定：Application 先 `PushOverlay(ImGuiLayer)`，用户再 `PushLayer(Example3D)`，故 Example3D 为层 0、先被遍历，其相机被使用。

3. **提交与 Flush**
   - 只有实现 **IDrawable** 的 Layer 才会在遍历时被 `SubmitTo(RenderQueue)`。
   - 3D 几何由 **SubmitMesh** 入队，在 **Flush3D()** 中通过 `Renderer3D::Submit(VAO, indexCount, transform, color)` 绘制；2D 由 **Flush2D()** 的 **DrawQuad** 绘制。
   - 若 SceneLayer 的 Scene 无实体、或实体无 MeshComponent，则 Flush3D 不会绘制任何东西。

4. **背面剔除**
   - `SetCullFace(true, true)` 表示剔除“背面”、正面为 CCW。若立方体三角形绕序与预期相反，所有面可能被剔除导致灰屏。可暂时 **SetCullFace(false, true)** 验证；若出现几何再修正网格绕序并恢复剔除。

5. **顶点布局与 Shader**
   - 默认 2D/3D Shader 要求：`location 0 = vec3 a_Position`，`location 1 = vec4 a_Color`，stride 7 floats。VAO 的 `AddVertexBuffer` 须按此布局设置 attribute 0 和 1。

6. **架构差异：为何“旧版可画、新版灰屏”**
   - **旧版**：Layer 在 **OnUpdate 内直接**调用 `Renderer2D::BeginScene(m_Camera)` → `DrawQuad(...)` → `EndScene()`，即“谁画谁绑相机、同一帧内立即绘制”，无统一队列。
   - **新版**：Application 先收集所有 IDrawable 的 **SubmitTo(queue)**，再**统一**取第一个 IProvidesCamera、一次 **BeginScene(cam)** 后 **Flush2D/Flush3D**。若在**仅有 3D 命令**时仍先调用 **Renderer2D::BeginScene** 再 Flush3D，会先绑定 2D 着色器与 2D 状态，再切到 3D；若存在全局/管线状态未正确隔离，会导致 3D 不绘制或灰屏。
   - **修复**：仅当 `Has2DCommands()` 时执行 `Renderer2D::BeginScene`/Flush2D/EndScene；仅当 `Has3DCommands()` 且相机为 PerspectiveCamera 时执行 `Renderer3D::BeginScene`/Flush3D/EndScene，避免无 2D 时绑定 2D 着色器干扰 3D。

---

## 十、依赖方向（禁止反向依赖）

```
Application (Core)
    │
    ├──► RendererAPI (Platform)     Application 可调用 RendererAPI
    │         ▲
    │         │ 实现
    │    Backends/OpenGL_GLFW
    │
    └──► Layer → Scene / IDrawable → RenderQueue
             │
             └──► Application 调用 RenderQueue::Flush2D/Flush3D → Renderer2D/Renderer3D → RendererAPI
```

**原则**：Renderer 层不包含 Platform/Backends 头文件；通过 RendererAPI 等抽象接口与底层通信。

---

## 十、对外接口（Renderer 模块）

### 10.1 统一头文件

```cpp
#include "Renderer/RendererModule.h"
```

包含：`Renderer`、`Camera`、`Renderer2D`、`Renderer3D`、`Scene`、`Material`。

### 10.2 初始化与关闭

- **Renderer::Init()**：由 `Application` 在 `RendererAPI::Init()` 之后调用（已接入 `Application` 构造）。
- **Renderer::Shutdown()**：由 `Application` 析构调用。

### 10.3 可绘制层写法（Layer 内）

Layer 实现 **IDrawable**（及可选 **IProvidesCamera**）：逻辑在 OnUpdate，提交在 SubmitTo；Application 统一收集队列后 Sort 并 Flush。简单示例（直接提交四边形）或持有 Scene 并 `scene->SubmitTo(queue)`，见 SandBox Example3DLayer。

```cpp
class MyLayer : public Ehu::Layer, public Ehu::IDrawable, public Ehu::IProvidesCamera {
    void OnUpdate(const Ehu::TimeStep& ts) override { /* 仅逻辑，不调用 BeginScene/Submit/EndScene */ }
    void SubmitTo(Ehu::RenderQueue& queue) const override {
        queue.SubmitQuad({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.8f, 0.2f, 0.3f, 1.0f });
    }
    Ehu::Camera* GetCamera() const override { return &m_Camera; }
};
```

- **Sorting**：RenderQueue::Sort() 不透明按 LayerIndex → MaterialKey → SortKey 升序（Front-to-Back），透明按 LayerIndex → SortKey 降序（Back-to-Front）；Flush 时透明段自动 SetBlend(true)。

### 10.4 按需包含

若只需部分接口，可单独包含：

- `Renderer/Renderer.h` — Renderer::Init / Shutdown
- `Renderer/Camera/Camera.h` — OrthographicCamera、PerspectiveCamera
- `Renderer/Drawable.h` — IDrawable、IProvidesCamera
- `Renderer/RenderQueue.h` — RenderQueue、DrawCommand2D/3D
- `Renderer/Renderer2D.h`、`Renderer/Renderer3D.h` — 2D/3D 渲染
- `Renderer/SceneLayer.h` — SceneLayer（引擎提供的场景渲染层）
- `Scene/Scene.h` — Scene、SceneNode（场景模块，纯数据）
- `Scene/SceneEntity.h` — SceneEntity、MeshComponent、SpriteComponent
- `Renderer/Material/Material.h` — Material、MaterialProperties

---

## 十一、数据流（简要）

```
Phase1: Layer::OnUpdate → SceneLayer 对 GetActivatedScenes() 逐 scene->OnUpdate(timestep)
Phase2: Layer::SubmitTo → SceneLayer 对激活场景中 GetRenderLayer()==this 的实体打包入 queue
Phase3: queue.Sort()（Layer→MaterialKey→深度/透明 Back-to-Front）→ BeginScene(cam) → Flush2D/Flush3D → EndScene()
```

---

## 十二、文档索引

| 文件 | 内容 |
|------|------|
| [ARCHITECTURE.md](ARCHITECTURE.md) | 引擎整体模块依赖与分层 |
| [TODO.md](TODO.md) | 渲染相关待办（Render API、Renderer、Entity、Queue、Sorting） |
| [README.md](README.md) | 项目说明与构建 |

---

*Renderer 层调度与 Render API（VertexBuffer、Shader、VertexArray、DrawIndexed）已实现并接入 Application 与 SandBox 示例。*
