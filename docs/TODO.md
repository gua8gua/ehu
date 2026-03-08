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


| 功能项             | 说明                                                                       | 状态  |
| --------------- | ------------------------------------------------------------------------ | --- |
| **Timestep**    | 帧时间步长，已接入 Layer::OnUpdate(TimeStep)；可补充 GetSeconds/GetMilliseconds 等便捷接口 | 已接入 |
| **Timer**       | 计时器工具（参考 Hazel Core），用于性能统计或延迟逻辑                                         | 待完成 |
| **UUID**        | 全局唯一 ID，用于实体、资源引用与序列化（Core/UUID.h/.cpp，64 位）                         | 已接入 |
| **FileSystem**  | 文件系统路径与基础操作（Core/FileSystem.h/.cpp，基于 std::filesystem）                     | 已接入 |
| **Ref / Scope** | 智能指针别名（Core/Ref.h：Ref=shared_ptr、Scope=unique_ptr，CreateRef/CreateScope）        | 已接入 |


---

## 二、渲染器层（Renderer）


| 功能项                | 说明                                                                 | 状态   |
| ------------------ | ------------------------------------------------------------------ | ---- |
| **Renderer2D 批处理** | 四边形、纹理四边形、子纹理已接入；多四边形合并批处理为后续扩展                                   | 部分完成 |
| **纹理与子纹理**         | Texture 抽象与 OpenGL 实现；SubTexture2D + CreateFromCoords 网格图集已实现                    | 已完成  |
| **文本渲染**           | 最小实现：Font 占位图集 + Renderer2D::DrawText API（ASCII）；TTF 图集可后续接 stb_truetype   | 部分完成 |
| **Framebuffer**    | 抽象 + OpenGL 实现已存在，可与 BeginRenderPass/EndRenderPass 配合用于离屏渲染与视口纹理           | 已完成  |
| **UniformBuffer**  | 抽象 + OpenGL 实现（Bind(slot)、SetData），可向着色器传递 uniform 块（如相机、灯光）               | 已完成  |
| **RenderCommand**  | 渲染命令队列封装（清屏、视口、Draw 等），与现有 RenderQueue 协同或统一                              | 可选   |
| **正交相机控制器**        | OrthographicCameraController（平移、缩放、旋转，SetMoveDelta/SetZoomDelta），供 2D 层使用   | 已完成  |
| **EditorCamera**   | 弧球/轨道相机最小实现（透视投影、SetRotationDelta/SetZoomDelta、SetViewportSize），供编辑器视口使用 | 已完成  |


---

## 三、场景与实体（Scene）


| 功能项                     | 说明                                                 | 状态   |
| ----------------------- | -------------------------------------------------- | ---- |
| **Scene / SceneEntity** | 场景图与实体已存在；可扩展按名/UUID 查找、实体复制                       | 部分完成 |
| **组件与序列化**              | 组件化描述（Tag、Transform、Sprite、Camera 等）与场景序列化（如 YAML） | 待完成  |
| **SceneSerializer**     | 场景保存/加载，与项目资源路径配合                                  | 待完成  |


---

## 四、平台与后端（Platform / Backends）


| 功能项                 | 说明                                        | 状态  |
| ------------------- | ----------------------------------------- | --- |
| **GraphicsContext** | 图形上下文抽象（当前由 GLFW/GLAD 隐式使用），便于多窗口或多后端     | 可选  |
| **Shader 文件加载**     | 当前以源码创建为主；支持从文件路径加载顶点/片段着色器               | 待完成 |
| **多平台**             | Linux/macOS 窗口与输入（在保留 Backends 抽象前提下增加实现） | 待完成 |


---

## 五、ImGui 与工具


| 功能项           | 说明                                             | 状态  |
| ------------- | ---------------------------------------------- | --- |
| **ImGui 多视口** | 当前禁用 ViewportsEnable 以避免清屏问题；若需多窗口可单独研究渲染路径    | 暂缓  |
| **调试/统计面板**   | 帧率、Draw Call 数、渲染器统计等（可放在 ImGui 层或独立 Debug 模块） | 待完成 |


---

## 六、扩展方向（参考 Hazel，非短期必做）


| 功能项       | 说明                                               | 状态  |
| --------- | ------------------------------------------------ | --- |
| **脚本系统**  | C# 脚本运行时与引擎桥接（ScriptEngine、ScriptGlue），实体脚本组件    | 远期  |
| **物理 2D** | Box2D 集成与 Rigidbody2D、BoxCollider2D 等组件          | 远期  |
| **项目与资源** | Project 单例、项目配置、资产目录与路径解析（.hproj、AssetDirectory） | 远期  |
| **编辑器应用** | 基于 Ehu 的编辑器（场景层级、内容浏览器、运行/模拟等）                   | 远期  |


---

## 建议完成顺序

1. **渲染增强**：纹理与子纹理 → Framebuffer（视口渲染到纹理）→ 正交相机控制器，便于 2D 与编辑器视口。
2. **场景与数据**：UUID、组件化描述与 SceneSerializer，便于场景保存与加载。
3. **工具与调试**：Timer、FileSystem、Ref/Scope；调试/统计面板。
4. **扩展**：脚本、物理、项目配置与编辑器，按需求分阶段推进。

---

## 与 example-docs 的对应关系

- **功能概览**：上表采用「功能项 | 说明 | 状态」形式，对应 example-docs 中各模块的「组件 | 功能」表。
- **架构要点**：分层与依赖见 [ARCHITECTURE.md](ARCHITECTURE.md)；新增功能时保持「抽象在 Platform/Renderer，实现在 Backends」的约定。
- 具体模块的详细说明可参考 `docs/example-docs/Hazel/` 下 Core、Renderer、Scene、Platform 等 README。

