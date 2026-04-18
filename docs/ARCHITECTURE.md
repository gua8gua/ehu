# Ehu 引擎架构说明

本文档描述重构后的模块划分、分层原则与数据流，便于扩展与维护。

---

## 分层与目录对应

| 层级 | 目录 | 职责 | 依赖 |
|------|------|------|------|
| **核心** | Core/ | Application、Layer、LayerStack、Log、键鼠码 | 仅 Events、ehupch；不依赖 Platform/Backends 具体实现 |
| **事件** | Events/ | Event 类型与 EventDispatcher | Core/Core.h（BIT、EHU_API）、Core/KeyCodes 等 |
| **平台抽象** | Platform/ | GraphicsBackend、Window/Input/RendererAPI 接口与工厂 | Core（Log 等）、Events；不包含 GLFW/OpenGL 头文件 |
| **ECS** | ECS/ | Entity、World、TypeId、Components（Transform/Sprite/Mesh/Camera/Tag/Id/Script） | Core、glm；不依赖 Renderer |
| **场景** | Scene/ | Scene、SceneNode（纯数据）；Scene 持有 World，管理 Entity 与主相机 | Core、ECS、glm；不依赖 Renderer |
| **脚本** | Scripting/ | ScriptEngine（Mono 运行时门面）、ScriptRegistry（实例索引/句柄回收）、ScriptGlue（InternalCall） | Core、Scene、ECS；Mono 可选 |
| **渲染器** | Renderer/ | Camera、Renderer2D、SceneLayer、Material（平台无关） | Core、Platform、Scene、ECS；不包含 GLFW/OpenGL 头文件 |
| **项目与资源** | Project/ | Project 单例、ProjectConfig、ProjectSerializer；项目文件（.ehuproject）、资产根目录与场景集合 | Core、Scene、Core/FileSystem、Scene/SceneSerializer |
| **ImGui 集成** | ImGui/ | ImGuiLayer、ImGuiBackend 接口与 Create(backend) | Core、Platform、Events；不包含具体后端 impl |
| **后端实现** | Backends/OpenGL_GLFW/ | WindowsWindow、WindowsInput、ImGuiBackendGLFWOpenGL、OpenGLRendererAPI | Platform、Core（Application 用于 Input）、Events；包含 GLFW/OpenGL |

- **核心层**不直接包含任何底层 API；清屏等由 **RendererAPI** 抽象，在 Application 中通过 `RendererAPI::Get().Clear()` 调用。
- **平台层**只做“选择与接口”：`GetGraphicsBackend()`、`Window::Create`、`Input::Init`、`RendererAPI::Init`、`ImGuiBackend::Create` 按后端分支，具体实现全部在 **Backends** 下。

---

## Scripting 子系统边界（重点）

脚本模块分为两层，职责不同：

- **原生运行时层（C++）**：`Ehu/src/Ehu/Scripting/`
  - 负责 Mono 域、程序集加载、生命周期调度、InternalCall 绑定、托管实例回收。
- **托管 API 与业务脚本层（C#）**：`Scripting/CSharp/Ehu.ScriptCore` 与 `Scripting/CSharp/Game`
  - 负责 `InternalCalls` 声明封装与项目脚本逻辑实现。

核心边界原则：

1. **ECS 持久化数据和托管实例分离**  
   - `ScriptComponent.Instances` 保存可序列化数据（`InstanceId`、`ClassName`、`Fields`）。  
   - Mono 托管对象与 `GCHandle` 不进 ECS，由 `ScriptRegistry` 集中管理。
2. **Scene 只依赖脚本接口，不依赖具体实现**  
   - `Scene` 通过 `IScriptRuntime` 调用 `OnSceneUpdate/OnSceneFixedUpdate/...`，解耦 `ScriptEngine` 内部细节。
3. **脚本能力通过白名单开放**  
   - C# 只能通过 `ScriptGlue` 注册的 InternalCall 访问引擎功能，避免直接穿透底层容器。

---

## 模块依赖关系（概念）

```
EntryPoint (main)
    → Log::Init()
    → CreateApplication()  [用户实现]
    → Application::Run()

Application (Core)
    ├── Window::Create()        → Platform 工厂 → Backends/OpenGL_GLFW/WindowsWindow
    ├── Input::Init()           → Platform 工厂 → Backends/OpenGL_GLFW/WindowsInput
    ├── RendererAPI::Init()     → Platform 工厂 → Backends/OpenGL_GLFW/OpenGLRendererAPI
    ├── ImGuiLayer(GetGraphicsBackend())  → ImGui 工厂 → Backends/OpenGL_GLFW/ImGuiBackendGLFWOpenGL
    ├── LayerStack              → 各 Layer::OnUpdate / OnImGuiRender / OnEvent
    ├── Project::GetActive()    → 若存在，按 ProjectConfig.Scenes（Active 为 true）调用 ActivateScenesFromProject
    └── Run() 内清屏            → RendererAPI::Get().SetClearColor() / Clear()
```

- **GetGraphicsBackend()**：在 `Platform/GraphicsBackend.cpp`，可读环境变量 `EHU_GRAPHICS_BACKEND`，默认 `OpenGL_GLFW`。
- 新增后端时：在 Platform 各工厂与 `GetGraphicsBackend()` 中增加分支，并在 `Backends/` 下新增目录（如 `Backends/Vulkan_GLFW`）实现对应接口。

---

## 事件流

1. 底层回调（如 GLFW）在 **Backends** 中转为 `Ehu::Event` 子类，调用 `WindowData::EventCallback`。
2. 即 `Application::OnEvent`：先处理 `WindowCloseEvent`，再按层栈从顶到底派发，`e.Handled` 可截断。
3. 新增事件类型：在 `Events/` 增加类型与 `EventType`/类别，在对应 Backend 的窗口/输入实现中构造并调用 `EventCallback`。

---

## Scene 与 Layer：正交关系

**Scene（场景）** 和 **Layer（图层）** 是**正交**的，不是上下级：

- **Scene**（`Scene/` 模块）：持有 **ECS::World**，管理场景图（SceneNode）与实体；通过 `CreateEntity()` / `DestroyEntity(Entity)`、`GetWorld()` 管理实体；主相机为带 CameraComponent 的 Entity，`SetMainCamera(Entity)` / `GetMainCamera()`。Scene **不**实现 IDrawable；**SceneLayer**（Renderer 模块）对每个已激活 Scene 的 World 做 **RunCameraSync** 与按 **RenderFilterComponent.Channel** 过滤的 **RenderExtract**，向 RenderQueue 提交。
- **Layer** = “怎么做（逻辑与渲染顺序）”：负责每帧逻辑（OnUpdate）；若使用场景，可继承 **SceneLayer** 并重写 **OnUpdateScene(TimeStep)**。用户通过 `Scene::CreateEntity()` + `AddComponent` 创建实体，在 OnUpdate 或逻辑层用 `GetWorld().GetComponent<TransformComponent>(e)` 等更新状态。

**实体与组件**：实体为 **Entity** 句柄；组件为纯数据（TransformComponent、SpriteComponent、MeshComponent、CameraComponent、TagComponent、IdComponent、ScriptComponent），定义在 `ECS/Components.h`。变换通过 TransformComponent 的 SetPosition/SetScale/SetRotation 与 GetWorldMatrix() 使用；脚本数据通过 `ScriptComponent.Instances[]`（`InstanceId + ClassName + Fields`）持久化，支持同实体多脚本。

---

## 层栈顺序与 Layer 所有权

- **PushLayer(layer)**：插入到当前“层”区间末尾。
- **PushOverlay(overlay)**：压到栈尾（最顶层）。
- 渲染/更新：从栈底到栈顶；事件：从栈顶到栈底。ImGui 作为 Overlay 压栈顶层。
- **Layer 所有权**：`PushLayer` / `PushOverlay` 传入的裸指针交给 **LayerStack** 管理，调用方不得再 `delete`。Application 析构时先调用 `m_LayerStack.Clear()` 统一 OnDetach 并 delete 各层，再 `Input::Shutdown()`、Renderer/RenderContext::Shutdown()，从而控制析构顺序（Layer 销毁早于 Input 等）。

---

## 图形/窗口后端选择与渲染抽象

- **目的**：核心与底层解耦；通过“环境选择”决定使用哪套窗口/输入/渲染/ImGui 实现。
- **实现要点**：
  - **Platform/GraphicsBackend**：枚举与 `GetGraphicsBackend()`。
  - **Platform/Window、Input、RendererAPI**：抽象接口 + 工厂（.cpp 内按 backend 分支并包含对应 Backends 头文件）。**Input** 由 Application 构造时 `Input::Init()` 按当前 GraphicsBackend 创建实例、析构时 `Input::Shutdown()` 释放；禁止在 Application 创建前或 Shutdown 后使用 Input。
  - **ImGui/ImGuiBackend**：抽象接口 + `Create(GraphicsBackend)`，具体实现在 **Backends/OpenGL_GLFW/ImGuiBackendGLFWOpenGL**。
  - 主循环清屏：**Application** 只调用 `RendererAPI::Get().SetClearColor()` 与 `Clear()`，由 **OpenGLRendererAPI** 内部调用 `glClearColor`/`glClear`。
- **扩展**：在 `GetGraphicsBackend()` 与各工厂中增加新 backend 分支，在 `Backends/` 下实现新目录即可。

---

## ECS 与场景实体

- **ECS 模块**（`ECS/`）：**Entity** 为句柄（id + generation）；**World** 提供 CreateEntity、DestroyEntity、AddComponent/RemoveComponent/GetComponent/HasComponent、`Each<C...>(func)` 按组件组合迭代；**TypeId** 提供 `GetTypeId<T>()` 供存储与查询；**Components** 定义 Transform、Sprite、Mesh、Camera、Tag、Id、Script 等组件。
- **Scene** 持有 World，CreateEntity 时默认添加 IdComponent 与 TagComponent；主相机为 Entity + TransformComponent + CameraComponent，GetMainCamera() 返回该实体的 Camera*。
- **SceneLayer::SubmitTo**：对每个已激活 Scene 先 **RunCameraSync(World)**（将 Transform 同步到 CameraComponent.Camera），再按 `World.Each<Transform, Sprite/Mesh>` 并根据 `RenderFilterComponent.Channel` 与 `SceneLayer.RenderChannel` 匹配后提交到 RenderQueue。
- **脚本更新链**：`Scene::OnUpdate`（Play）调用 `ScriptEngine::OnSceneUpdate`；`ScriptEngine` 读取 `ScriptComponent.Instances[]` 并通过 `ScriptRegistry` 管理 `Scene + Entity + InstanceId` 对应托管实例（`GCHandle`）。

---

## CMake 结构概要

- **Ehu/src/Ehu/CmakeLists.txt**：`ehupch.cpp` + `add_subdirectory(Core, Events, Platform, ECS, Scene, Renderer, ImGui, Project, Editor, Scripting, Backends)`。
- **Core / Events / Platform / Scene / Renderer / ImGui**：各目录下 CMakeLists 显式列出本模块的源文件，`target_sources(EhuLib PRIVATE ...)`。
- **Backends/CMakeLists.txt**：`add_subdirectory(OpenGL_GLFW)`；**Backends/OpenGL_GLFW** 下列出该后端所有 .cpp/.h。
- 预编译头与 vendor 在 **Ehu/CMakeLists.txt** 中统一配置；各子目录仅增加源与 include 路径。

---

## 扩展建议

- **新后端**：实现 `Window`、`Input`、`RendererAPI`、`ImGuiBackend` 子类，放入新目录 `Backends/XXX`，并在 Platform 与 ImGui 的工厂中增加分支。
- **新事件**：在 `Events/` 增加类型，在对应 Backend 的窗口/输入实现中派发。
- **渲染管线**：可在现有 `RendererAPI` 上扩展（如 DrawIndexed），或新增 Renderer/Command 层，由 Layer 提交、Application 统一 flush。
