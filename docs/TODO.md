# Ehu 引擎待办清单

本文档对照 Cherno 系列教程与当前代码，列出**缺失模块**、**层级/命名不清**及**建议修复**，便于按优先级补齐。

**说明**：项目已重构为 **Core / Events / Platform / ImGui / Backends** 分层；底层相关代码集中在 **Backends/OpenGL_GLFW**，核心与平台抽象不直接依赖 GLFW/OpenGL。详见 [ARCHITECTURE.md](ARCHITECTURE.md)。

---

## 一、缺失模块

### 1. 时间步长（Timestep / Delta Time）


| 状态  | 说明                                                                        |
| --- | ------------------------------------------------------------------------- |
| 缺失  | `Layer.h` 中已注释 `//#include "Timestep.h"`，当前 `OnUpdate()` 无参数，无法做与帧率无关的更新。 |


**待办：**

- 新增 `Timestep.h` / `Timestep.cpp`（或仅头文件），封装 delta 时间（如 `float GetSeconds()`）。
- 主循环中每帧计算 dt（如用 `glfwGetTime()` 或 `std::chrono`），传入 `Application::Run()` 内层。
- 将 `Layer::OnUpdate()` 改为 `OnUpdate(Timestep dt)`，并在 `Application::Run()` 中调用时传入 dt。

---

### 3. 渲染架构（Render API 与 Renderer 分层）

参考渲染架构图（Two Sides：Renderer 与 Render API），分为 **Render API（API/平台相关）** 与 **Renderer（API/平台无关）** 两层。

#### 3.1 Render API 层（API/platform specific）

| 模块             | 状态   | 说明                          |
| -------------- | ---- | --------------------------- |
| Render Context | **已做** | RenderContext::Init/Shutdown/GetAPI，SetCurrentWindow/SwapBuffers；Application 使用其初始化与 Present |
| Swap chain     | **部分** | Window::SwapBuffers 抽象，Present 由 RenderContext::SwapBuffers 或 Window::OnUpdate 触发 |
| Framebuffer    | **已做** | 帧缓冲（颜色、深度、模板），Platform + OpenGL，支持 MRT |
| Vertex Buffer  | **已做** | 顶点缓冲，Platform 抽象 + OpenGL 实现     |
| Index Buffer   | **已做** | 索引缓冲，Platform 抽象 + OpenGL 实现     |
| VertexArray    | **已做** | 管理 VB/IB 绑定与布局，Platform + OpenGL  |
| Texture        | **已做** | Texture2D：Create/CreateFromFile/Bind，Platform + OpenGL（文件需 EHU_USE_STB_IMAGE） |
| Shader         | **已做** | 着色器抽象（源码、uniform、bind），Platform + OpenGL |
| States         | **已做** | RendererAPI：SetDepthTest/SetBlend/SetCullFace/SetViewport，Clear(flags) |
| Pipelines      | **已做** | Pipeline：Shader+VAO+状态，Bind+DrawIndexed，Platform + OpenGL |
| Render passes  | **已做** | RendererAPI::BeginRenderPass/EndRenderPass，绑定 FBO/解绑 |
| DrawIndexed    | **已做** | RendererAPI::DrawIndexed，OpenGL 已实现   |


**待办：**

- [x] 抽象 `VertexBuffer`、`IndexBuffer`（创建、绑定、布局）。
- [x] 抽象 `Shader`（源码/文件加载、uniform、bind）。
- [x] 抽象 `VertexArray`，管理顶点属性与 VB/IB 绑定。
- [x] 抽象 `Texture`（2D：Create/CreateFromFile/Bind/SetData，OpenGL 已实现）。
- [x] 抽象 `Framebuffer`，支持 MRT、深度/模板附件，OpenGL 已实现。
- [x] `States`：RendererAPI 扩展深度/混合/裁剪/视口；`Render passes`：BeginRenderPass/EndRenderPass。
- [x] 抽象 `RenderContext`（Init/Shutdown/GetAPI、SetCurrentWindow/SwapBuffers）；抽象 `Pipeline`（Shader+VAO+状态，Bind+DrawIndexed），OpenGL 已实现。
- [x] 扩展 `RendererAPI`（`DrawIndexed`、Clear(flags)），OpenGL 已实现。
- [x] 以上新项已做 OpenGL 实现，接口在 `Platform/`，实现在 `Backends/OpenGL_GLFW/`。

#### 3.2 Renderer 层（API/platform agnostic）

| 模块               | 状态   | 说明                         |
| ---------------- | ---- | -------------------------- |
| 2D Renderer      | **已做** | Renderer2D：BeginScene/DrawQuad/EndScene |
| 3D Renderer      | 占位   | Renderer3D 接口占位，待前向渲染实现       |
| Scene Graph      | **已做** | Scene / SceneNode：层次、变换传播      |
| Sorting          | 缺失   | 渲染排序（透明度、队列优化等）            |
| Culling          | 缺失   | 视锥体剔除、遮挡剔除等                |
| Materials        | **部分** | Material + MaterialProperties，Bind 待接 Shader |
| LOD              | 缺失   | 细节级别，按距离调整模型精度             |
| Animation        | 缺失   | 骨骼/顶点动画                    |
| Camera           | **已做** | OrthographicCamera、PerspectiveCamera  |
| VFX              | 缺失   | 粒子系统等视觉特效                  |
| PostFX           | 缺失   | 后期处理（模糊、景深、色调校正等）          |
| 其他               | 缺失   | 反射、环境光遮蔽等                  |


**待办：**

- [x] 实现 `OrthographicCamera`（2D）：位置、旋转、投影矩阵，与 glm 集成。
- [x] 实现 `PerspectiveCamera`（3D），支持透视投影（占位）。
- [x] 2D Renderer：基于 VertexArray + Shader 的 DrawQuad。
- [ ] 3D Renderer：前向渲染基础，支持多物体、多材质。
- [x] Scene Graph：SceneNode 节点层次、变换传播。
- [ ] Materials：材质属性与 Shader 绑定（当前 Bind 占位）。
- [ ] Sorting / Culling：按队列排序、视锥体剔除。
- [ ] PostFX：全屏后处理管线（模糊、Bloom、色调映射等）。
- [ ] VFX：粒子系统基础。
- [ ] LOD、Animation：后续迭代。

---

### 4. 窗口与视口（WindowResize / 视口）


| 状态  | 说明                                                    |
| --- | ----------------------------------------------------- |
| 部分  | `WindowResizeEvent` 已存在并在 GLFW 中触发，但 Application 未处理。 |


**待办：**

- 在 `Application::OnEvent` 中 `Dispatch<WindowResizeEvent>`，更新视口或相机宽高比（如 `glViewport`、相机 aspect）。
- 避免窗口拉伸时渲染变形或未随窗口 resize。

---

### 5. 断言开关（EHU_ENABLE_ASSERTS）


| 状态  | 说明                                                                               |
| --- | -------------------------------------------------------------------------------- |
| 缺失  | `Core.h` 中 `EHU_ASSERT` / `EHU_CORE_ASSERT` 依赖 `EHU_ENABLE_ASSERTS`，若未定义则断言为空操作。 |


**待办：**

- 在 CMake 的 Debug 配置中定义 `EHU_ENABLE_ASSERTS`（或在与 Debug 相关的配置中统一开启）。
- 在文档中说明：Debug 下默认开启断言，Release 可关闭。

---

## 二、层级与命名不清晰

### 1. ImGui 层：Layer 与 Overlay 角色


| 问题   | 说明                                                                                                                |
| ---- | ----------------------------------------------------------------------------------------------------------------- |
| 角色混用 | 当前用 `PushLayer(m_ImGuiLayer)` 将 ImGui 作为普通层压入。Cherno 中通常将 ImGui 作为 **Overlay** 压在最顶层，便于“最后绘制、最后收事件”并正确做 hover 遮挡。 |


**待办：**

- 将 ImGui 改为 `PushOverlay(m_ImGuiLayer)`（即压入层栈顶层）。
- 确认事件派发顺序：Overlay 先收事件，若 ImGui 需要捕获（如 hover 时）可设 `Handled`，游戏层再收。
- 确认渲染顺序：先各 Layer `OnUpdate` 绘制场景，再 Overlay 的 ImGui，与当前 `Begin/OnImGuiRender/End` 的用法一致即可。

---

### 2. API 命名：PushOverLayer vs PushOverlay


| 问题  | 说明                                                                             |
| --- | ------------------------------------------------------------------------------ |
| 不一致 | `Application` 中为 `PushOverLayer`，`LayerStack` 中为 `PushOverlay`。语义相同但拼写不统一，易混淆。 |


**待办：**

- 统一为 `PushOverlay`（与 LayerStack 一致），或统一为 `PushOverLayer`（与 Layer 对称）。建议改为 `PushOverlay`，并重命名 `Application::PushOverLayer` → `PushOverlay`，更新 SandBox 与文档。

---

### 3. KeyTypedEvent：KeyCode 与 codepoint 语义


| 问题      | 说明                                                                                                                       |
| ------- | ------------------------------------------------------------------------------------------------------------------------ |
| 类型/语义错误 | `KeyTypedEvent` 构造函数使用 `KeyCode`（按键码），而 GLFW 的 `glfwSetCharCallback` 提供的是 **Unicode codepoint**（字符码）。两者语义不同：按键码 vs 输入字符。 |


**待办：**

- 将 `KeyTypedEvent` 的成员与参数改为“字符码”类型（如 `unsigned int codepoint` 或 `uint32_t`），与 GLFW 一致。
- 重命名或注释：`GetKeyCode()` 若保留可改为 `GetCodepoint()` 或单独提供 `GetCodepoint()`，避免与 KeyPressed/KeyReleased 的 KeyCode 混用。
- 更新所有使用 `KeyTypedEvent` 的代码（若有）及文档。

---

### 4. Input 平台实例化方式


| 问题       | 说明                                                                                                                      |
| -------- | ----------------------------------------------------------------------------------------------------------------------- |
| 耦合与初始化顺序 | `Input::s_instance` 在 `WindowsInput.cpp` 中直接赋值为 `new WindowsInput()`，平台与引擎耦合在一处，且静态初始化顺序未定义，若在 `Application` 之前使用可能未就绪。 |


**待办：**

- 考虑在 `Application` 构造或 `Log::Init()` 之后显式调用 `Input::SetInstance(...)` 或类似初始化，由入口点按平台选择实现类。
- 或保留当前方式但在文档中明确：Input 仅在 Application 创建后使用，避免静态初始化顺序问题。

---

### 5. LayerStack 与 Application 的职责边界


| 问题     | 说明                                                                                                                                                                                                                      |
| ------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 所有权与析构 | `Application::PushLayer/PushOverlay` 将裸指针交给 `LayerStack`，`LayerStack` 析构时会 `delete` 各层；但 `ImGuiLayer` 在 `Application` 中 new 且未在析构中 delete，由 LayerStack 统一 delete，这是合理的。建议在文档中明确“Layer 所有权归 LayerStack”，避免多处 delete 或泄漏。 |


**待办：**

- 在架构文档中写明：Layer 由 Application 传入后，生命周期由 LayerStack 管理，Application 析构时不应再 delete 已 Push 的 Layer。
- 可选：接口改为 `std::unique_ptr<Layer>` 或明确注释“调用方不再拥有该指针”。

---

## 三、建议的完成顺序（参考）

1. **先修语义与命名**：KeyTypedEvent codepoint、PushOverlay 命名、ImGui 改为 Overlay。
2. **再补时间与窗口**：Timestep、Layer::OnUpdate(dt)、WindowResize 处理、EHU_ENABLE_ASSERTS。
3. **Render API 层（自底向上）**：
  - VertexBuffer / IndexBuffer / VertexArray → Shader → Texture
  - 扩展 RendererAPI（DrawIndexed、States）
  - Framebuffer、Render passes（可选，为 PostFX 打基础）
4. **Renderer 层（API 无关）**：
  - Camera（Orthographic → Perspective）
  - 2D Renderer 基础 → Materials → 3D 前向渲染
  - Scene Graph、Sorting、Culling
5. **进阶**：PostFX、VFX、LOD、Animation。

---

## 四、文档与规范

- 完成上述修改后，同步更新 `docs/ARCHITECTURE.md` 与 `docs/README.md`（事件类型、API 命名、层栈顺序、Input 初始化约定）。
- 若新增 Renderer/Platform 目录，在 `docs/ARCHITECTURE.md` 中补充模块图与依赖关系。

---

*本文档随实现进度更新，完成一项可在对应 `[ ]` 改为 `[x]`。*