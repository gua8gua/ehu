# Ehu 待办与功能清单

本文档按 `docs/example-docs` 的案例格式，列出引擎要完成的功能项，便于按模块与优先级推进。参考 Hazel 对应模块（Core、Renderer、Scene、Platform 等）的能力进行规划。

---

## 文档结构（与 example-docs 对应）

| 路径                                     | 说明                          |
| -------------------------------------- | --------------------------- |
| [README](../README.md)                 | 项目说明、结构、构建、架构与使用            |
| [ARCHITECTURE](ARCHITECTURE.md)        | 模块依赖、事件流、层栈与扩展              |
| [example-docs](example-docs/README.md) | Hazel 引擎文档总览（功能概览与架构要点格式参考） |

---

## 一、核心层（Core）

| 功能项             | 说明                                                                       | 状态   |
| --------------- | ------------------------------------------------------------------------ | ---- |
| **Timestep**    | 帧时间步长，已接入 Layer::OnUpdate(TimeStep)；可补充 GetSeconds/GetMilliseconds 等便捷接口 | 已接入 |
| **Timer**       | 计时器工具（Core/Timer.h），用于性能统计或延迟逻辑；Application 中已用于 UpdateMs/RenderSubmitMs | 已完成 |
| **UUID**        | 全局唯一 ID，用于实体、资源引用与序列化（Core/UUID.h/.cpp，64 位）                         | 已接入 |
| **FileSystem**  | 文件系统路径与基础操作（Core/FileSystem.h/.cpp，基于 std::filesystem）                     | 已接入 |
| **Ref / Scope** | 智能指针别名（Core/Ref.h：Ref=shared_ptr、Scope=unique_ptr，CreateRef/CreateScope）        | 已接入 |

---

## 二、渲染器层（Renderer）

| 功能项                | 说明                                                                 | 状态   |
| ------------------ | ------------------------------------------------------------------ | ---- |
| **Renderer2D 批处理** | 纯色与带纹理/子纹理四边形均已批处理，按纹理 ID 断批；CreateBatch2DTextured，EndScene 时 Flush 两路批 | 已完成  |
| **纹理与子纹理**         | Texture 抽象与 OpenGL 实现；SubTexture2D + CreateFromCoords 网格图集已实现                    | 已完成  |
| **文本渲染**           | 最小实现：Font 占位图集 + Renderer2D::DrawText API（ASCII）；TTF 图集可后续接 stb_truetype   | 部分完成 |
| **Framebuffer**    | 抽象 + OpenGL 实现已存在，可与 BeginRenderPass/EndRenderPass 配合用于离屏渲染与视口纹理           | 已完成  |
| **UniformBuffer**  | 抽象 + OpenGL 实现（Bind(slot)、SetData），可向着色器传递 uniform 块（如相机、灯光）               | 已完成  |
| **RenderCommand**  | 渲染命令队列封装（清屏、视口、Draw 等），与现有 RenderQueue 协同或统一                              | 可选   |
| **正交相机控制器**        | OrthographicCameraController（平移、缩放、旋转，SetMoveDelta/SetZoomDelta），供 2D 层使用   | 已完成  |
| **EditorCamera**   | 弧球/轨道相机最小实现（透视投影、SetRotationDelta/SetZoomDelta、SetViewportSize），供编辑器视口使用 | 已完成  |

---

## 三、场景与实体（Scene）

**规范与约定**：见 [.cursor/rules/ehu-scene.mdc](../.cursor/rules/ehu-scene.mdc)（ECS/Scene/SceneLayer 职责、实体与组件、主相机、渲染提取、SceneNode、待办约定）。

| 功能项                     | 说明                                                                 | 状态   |
| ----------------------- | ------------------------------------------------------------------ | ---- |
| **Scene / SceneEntity** | 场景图与实体已存在；按名查找（FindEntityByName）、按 UUID 查找（FindEntityByUUID）、实体复制（DuplicateEntity）已实现 | 已完成  |
| **组件与序列化**              | 组件化描述（Tag、Transform、Sprite、Camera 等）；SceneSerializer 序列化 Id/Tag/Transform/Sprite/Camera（Ortho/Persp 参数），主相机 UUID 与场景持有相机 | 已完成  |
| **SceneSerializer**     | 场景保存/加载（Serialize/Deserialize），路径与读写通过 FileSystem；主相机 UUID 保存与恢复               | 已完成  |

---

## 四、平台与后端（Platform / Backends）

| 功能项                 | 说明                                        | 状态  |
| ------------------- | ----------------------------------------- | --- |
| **GraphicsContext** | 图形上下文抽象（当前由 GLFW/GLAD 隐式使用），便于多窗口或多后端     | 可选  |
| **Shader 文件加载**     | Shader::CreateFromFile(vertexPath, fragmentPath)，FileSystem::ReadTextFile + 现有编译逻辑 | 已完成  |
| **多平台**             | Linux/macOS 窗口与输入（在保留 Backends 抽象前提下增加实现） | 待完成 |

---

## 五、ImGui 与工具

**规范与约定**：见 [.cursor/rules/ehu-imgui-tools.mdc](../.cursor/rules/ehu-imgui-tools.mdc)（ImGui 集成、事件阻塞、多视口暂缓、调试/统计面板约定）。

| 功能项           | 说明                                                                 | 状态   |
| ------------- | ------------------------------------------------------------------ | ---- |
| **ImGui 多视口** | 当前禁用 ViewportsEnable 以避免清屏问题；若需多窗口可单独研究渲染路径                | 暂缓   |
| **调试/统计面板**   | DebugLayer 已实现：FPS、Delta、Draw Calls 2D/3D、Triangles 2D/3D；Application 默认 PushOverlay(DebugLayer) | 已完成 |

---

## 六、扩展方向（参考 Hazel，非短期必做）

| 功能项       | 说明                                               | 状态  |
| --------- | ------------------------------------------------ | --- |
| **脚本系统**  | C# 脚本运行时与引擎桥接（ScriptEngine、ScriptGlue），实体脚本组件    | 远期  |
| **物理 2D** | Box2D 集成与 Rigidbody2D、BoxCollider2D 等组件          | 远期  |
| **项目与资源** | Project 单例、项目配置（ProjectConfig）、项目文件（.ehuproject）、资产目录与路径解析（AssetDirectory） | 已完成 |
| **编辑器应用** | 基于 Ehu 的编辑器（场景层级、内容浏览器、运行/模拟等）                   | 远期  |

---

## 建议完成顺序（当前项目开发步骤）

按与 example-docs 比对后的优先级，建议按以下步骤推进：

| 步骤 | 内容 | 说明 |
| --- | --- | --- |
| 1 | **Shader 文件加载**（已完成） | Platform 层 Shader::CreateFromFile(vertexPath, fragmentPath)，FileSystem::ReadTextFile + 现有编译逻辑。 |
| 2 | **组件与序列化补全**（已完成） | SceneSerializer 序列化/反序列化 Camera（Ortho/Persp 参数）；Scene 持有反序列化相机（AddOwnedCamera/ClearOwnedCameras）；相机类增加投影参数 getter。 |
| 3 | **渲染增强**（已完成） | 纹理四边形/子纹理纳入 Renderer2D 批处理（CreateBatch2DTextured，按纹理指针断批，10 floats/顶点），EndScene 时 Flush 纯色批与带纹理批。 |
| 4 | **文本渲染增强（可选）** | Font + Renderer2D::DrawText 已有占位；可接 stb_truetype 生成图集或沿用现有最小实现，文档标注「部分完成」与后续选项。与 Hazel 文本/MSDF 对应，优先级低于 1–3。 |
| 5 | **工具与体验** | Timer 已在用，确认 Debug 面板统计与 FileSystem 使用无误；无缺则仅文档状态更新。收尾与文档一致性。 |
| 6 | **多平台 / 可选** | Linux/macOS 窗口与输入、GraphicsContext 抽象等，按需排期；RenderCommand 仍标为可选。与 example-docs Platform 扩展对应。 |

- **步骤 1–2**：近期必做。  
- **步骤 3–4**：渲染与内容增强。  
- **步骤 5**：状态与文档同步。  
- **步骤 6**：中长期或可选。

---

## 与 example-docs 的对应关系

- **功能概览**：上表采用「功能项 | 说明 | 状态」形式，对应 example-docs 中各模块的「组件 | 功能」表。
- **架构要点**：分层与依赖见 [ARCHITECTURE.md](ARCHITECTURE.md)；新增功能时保持「抽象在 Platform/Renderer，实现在 Backends」的约定。
- 具体模块的详细说明可参考 `docs/example-docs/Hazel/` 下 Core、Renderer、Scene、Platform 等 README。
