# Ehu 渲染架构

本文档梳理渲染子系统的构成：**平台无关部分**、**平台兼容部分**与**底层实现部分**，以及工作流程与各自保管的数据。

---

## 一、架构分层总览

| 层级 | 位置 | 职责 | 依赖 |
|------|------|------|------|
| **平台无关** | `Ehu/Renderer/` | 场景提交接口、渲染队列、2D/3D 调度、可绘制抽象 | Core、Camera、glm；仅通过 Platform 抽象调用 GPU |
| **平台兼容** | `Ehu/Platform/Render/`、`Platform/Backend/` | 渲染 API 抽象、上下文、资源工厂、后端选择 | Core；不包含具体 GL/Vulkan 头文件 |
| **底层实现** | `Ehu/Backends/OpenGL_GLFW/` | OpenGL/GLFW 具体实现 | Platform 接口、glad/GLFW |

---

## 二、平台无关部分（Renderer/）

与具体图形 API、窗口系统无关，只依赖“抽象接口”（RendererAPI、Shader、VertexArray 等）和数学/相机。

### 2.1 模块与职责

| 模块 | 文件 | 职责 |
|------|------|------|
| **Renderer** | `Renderer.h/cpp` | 渲染器层统一入口：`Init()` / `Shutdown()`，内部依次初始化/关闭 Renderer2D、Renderer3D。 |
| **Renderer2D** | `Renderer2D.h/cpp` | 2D 场景：`BeginScene(Camera)` / `EndScene()`，`DrawQuad(position, size, color)`；使用内置四边形几何与默认 2D Shader。 |
| **Renderer3D** | `Renderer3D.h/cpp` | 3D 场景：`BeginScene(PerspectiveCamera)` / `EndScene()`，`Submit(VertexArray*, indexCount, transform, color)`；使用默认 3D Shader，不持有网格。 |
| **RenderQueue** | `RenderQueue.h/cpp` | 收集 2D/3D 绘制命令，按层/相机/材质/深度排序，`Flush2D()` / `Flush3D()` / `FlushAll()` 统一提交到 Renderer2D/Renderer3D。 |
| **Drawable** | `Drawable.h` | `IDrawable::SubmitTo(RenderQueue&)`、`IProvidesCamera::GetCamera()`；Layer/SceneLayer 实现后向队列提交命令并提供相机。 |
| **SceneLayer** | `SceneLayer.h/cpp` | 从已激活 Scene 的 **ECS::World** 做 RunCameraSync，再按 **Transform+Sprite/Mesh+TagComponent**（Tag.RenderLayer==本层）迭代并 SubmitQuad/SubmitMesh 到 RenderQueue。 |

### 2.2 保管的数据（平台无关层）

| 所有者 | 数据 | 说明 |
|--------|------|------|
| **Renderer** | 无静态数据 | 仅调度 Init/Shutdown。 |
| **Renderer2D** | `SceneData*`（ViewProjectionMatrix）<br>`Shader*`（默认 2D）<br>`VertexArray*`（单位四边形） | 每帧 `BeginScene` 写入 ViewProjection；四边形几何与 Shader 在 Init 时创建。 |
| **Renderer3D** | `SceneData*`（ViewProjectionMatrix）<br>`Shader*`（默认 3D） | 每帧 `BeginScene` 写入 ViewProjection；不持有 VAO，由调用方传入。 |
| **RenderQueue** | `m_Commands2D` / `m_Commands3D`<br>`m_SortedOpaque2D/Transparent2D`<br>`m_SortedOpaque3D/Transparent3D`<br>`m_CurrentLayerIndex` | 每帧由各 Layer 提交后排序，Flush 时只读；Application 持有一个 `Scope<RenderQueue>`。 |

### 2.4 RenderCommand（可选，当前未实现）

**概念**：RenderCommand 是对“单次底层渲染操作”的封装，例如：清屏（SetClearColor + Clear）、设置视口（SetViewport）、开始/结束 RenderPass（BeginRenderPass/EndRenderPass）、单次 Draw（DrawIndexed）等。命令可入队后统一在合适时机执行，便于与多线程或延迟提交协同。

**与现有架构的关系**：

| 现有机制 | 与 RenderCommand 的关系 |
|----------|---------------------------|
| **RenderQueue** | 保管的是**场景级**绘制请求（2D Quad、3D Mesh、sortKey、相机等），按层/相机/材质排序后由 FlushAll 调用 Renderer2D/3D。RenderCommand 若实现，则是更底层的**API 级**命令（Clear、Viewport、DrawIndexed 等），可与 RenderQueue 并存：例如 FlushAll 内部对每条“逻辑绘制”生成对应 RenderCommand，再由命令队列统一提交给 RendererAPI；或由 Layer 直接提交清屏/视口等命令。 |
| **RendererAPI** | 当前由 Renderer2D/3D 直接调用 `RendererAPI::Get().Clear()`、`DrawIndexed()` 等。引入 RenderCommand 后，可改为“将调用封装为命令对象，压入队列，再由专门逻辑（如主线程或渲染线程）按序执行”，从而在保持同一套 RendererAPI 抽象的前提下，支持命令队列、批处理或多线程提交。 |

**设计要点（供后续实现参考）**：

- **命令类型**：枚举或变体，如 `Clear`、`SetViewport`、`BeginRenderPass`、`EndRenderPass`、`DrawIndexed`（含 VAO、count、管线状态等）。
- **执行时机**：可与当前每帧流程一致——在 FlushAll 之前/之后插入“执行命令队列”；或与 RenderQueue 的 Flush 合并为“先执行 Clear/Viewport/RenderPass，再按排序结果生成并执行 Draw 命令”。
- **与 RenderQueue 的协同**：不替代 RenderQueue 的“按层/相机排序”职责；RenderCommand 负责“把已确定的绘制与状态变更”以队列形式交给 RendererAPI 执行，二者分层清晰。

当前架构不依赖 RenderCommand 即可完成整帧渲染；若需命令队列、多线程渲染或更细粒度控制，再引入此模块。

---

### 2.5 与平台层的衔接

- Renderer2D / Renderer3D 在 **实现文件**（.cpp）中 `#include` Platform 的 `RendererAPI.h`、`Shader`、`VertexArray` 等，并调用：
  - `RendererAPI::Get().DrawIndexed(...)`、`SetBlend`、`SetDepthTest` 等；
  - `Shader::CreateDefault2D()` / `CreateDefault3D()`、`SetMat4` / `SetFloat4` 等；
  - `VertexArray::Create()`、`VertexBuffer::Create()`、`IndexBuffer::Create()`（仅 Renderer2D 建四边形时）。
- 头文件（.h）仅前向声明 `Shader`、`VertexArray`，不包含 Platform 路径，保持“接口上平台无关”。

---

## 三、平台兼容部分（Platform/Render/、Platform/Backend/）

定义与后端无关的抽象接口，以及按 `GraphicsBackend` 选择实现的工厂；不包含 OpenGL/Vulkan 等具体 API 头文件。

### 3.1 后端选择

| 模块 | 文件 | 职责 |
|------|------|------|
| **GraphicsBackend** | `Platform/Backend/GraphicsBackend.h/.cpp` | 枚举 `OpenGL_GLFW` 等；`GetGraphicsBackend()` 供各工厂分支使用（可读环境变量）。 |

### 3.2 渲染上下文与 API 抽象

| 模块 | 文件 | 职责 |
|------|------|------|
| **RenderContext** | `Platform/Render/RenderContext.h/.cpp` | `Init()` → `RendererAPI::Init()`；`Shutdown()`；`GetAPI()`；`SetCurrentWindow(Window*)`；`SwapBuffers()`（转发到当前 Window）。保管 `s_CurrentWindow`。 |
| **RendererAPI** | `Platform/Render/RendererAPI.h` + `.cpp` | 抽象接口：清屏（SetClearColor、Clear）、绘制（DrawIndexed）、状态（SetDepthTest、SetBlend、SetCullFace、SetViewport）、RenderPass（BeginRenderPass、EndRenderPass）。静态 `Init()`/`Shutdown()`/`Get()` 根据 GraphicsBackend 创建具体实现（如 OpenGLRendererAPI）。 |

### 3.3 资源抽象与工厂

所有资源均为抽象基类 + 静态 `Create*` 工厂；实现位于 Backends，工厂在 Platform 的 .cpp 中按 `GetGraphicsBackend()` 分支。

| 抽象 | 头文件 | 工厂方法 | 职责 |
|------|--------|----------|------|
| **Shader** | `Platform/Render/Resources/Shader.h` | `Create(vs, fs)`、`CreateDefault2D()`、`CreateDefault3D()` | Bind/Unbind；SetMat4/SetFloat4/SetFloat3/SetFloat/SetInt。 |
| **VertexArray** | `Resources/VertexArray.h` | `Create()` | Bind/Unbind；AddVertexBuffer、SetIndexBuffer；GetVertexBuffers/GetIndexBuffer。 |
| **VertexBuffer** | `Resources/VertexBuffer.h` | `Create(size)`、`Create(data, size)` | Bind/Unbind；SetData。 |
| **IndexBuffer** | `Resources/IndexBuffer.h` | `Create(indices, count)` | Bind/Unbind；GetCount。 |
| **Texture2D** | `Resources/Texture2D.h` | `Create(w, h, data)`、`CreateFromFile(path)` | GetWidth/Height/GetRendererID；SetData；Bind(slot)。 |
| **Framebuffer** | `Platform/Render/Framebuffer.h` | `Create(FramebufferSpec)` | Bind/Unbind；Resize；GetColorAttachmentRendererID；GetSpec。 |
| **Pipeline** | `Platform/Render/Pipeline.h` | `Create()` | SetShader/SetVertexArray、SetDepthTest/SetBlend/SetCullFace；Bind；DrawIndexed。 |

### 3.4 平台兼容层保管的数据

| 所有者 | 数据 | 说明 |
|--------|------|------|
| **RenderContext** | `s_CurrentWindow` | 用于 SwapBuffers。 |
| **RendererAPI** | `s_API`（单例实现指针） | Init 时按后端创建，Shutdown 时释放。 |
| **资源类** | 无全局状态 | 具体资源对象由调用方或 Renderer2D/Renderer3D 持有。 |

---

## 四、底层实现部分（Backends/OpenGL_GLFW/）

实现 Platform 定义的渲染接口，直接调用 OpenGL（glad）与 GLFW。

### 4.1 实现类与对应抽象

| 实现类 | 实现的抽象 | 说明 |
|--------|------------|------|
| **OpenGLRendererAPI** | RendererAPI | glClearColor、glClear、glDrawElements、深度/混合/背面剔除、glViewport、FBO 绑定。 |
| **OpenGLShader** | Shader | 编译 vs/fs，glUseProgram，uniform 设置。 |
| **OpenGLVertexArray** | VertexArray | VAO + 管理 VBO/IBO 绑定与布局。 |
| **OpenGLVertexBuffer** | VertexBuffer | VBO，glBufferData。 |
| **OpenGLIndexBuffer** | IndexBuffer | EBO，glBufferData。 |
| **OpenGLTexture2D** | Texture2D | 2D 纹理，创建/绑定/上传。 |
| **OpenGLFramebuffer** | Framebuffer | FBO，颜色/深度附件，Resize。 |
| **OpenGLPipeline** | Pipeline | 捆绑 Shader + VertexArray + 状态，Bind 后 DrawIndexed。 |

### 4.2 底层层保管的数据

- 每个 **OpenGL*** 对象持有对应的 GL 句柄（program、VAO、VBO、EBO、texture、FBO 等）。
- **OpenGLRendererAPI** 无持久数据，仅封装一次调用。
- 默认 2D/3D Shader 的源码或编译结果由 **OpenGLShader** 持有；几何由 **Renderer2D** 在 Init 时通过 `VertexArray::Create()` 等得到的是 OpenGL 实现，数据在 GPU 缓冲中。

---

## 五、工作流程

### 5.1 初始化顺序（Application 构造）

1. `Window::Create()` → 窗口与 OpenGL 上下文（由 Backend 创建）。
2. `Input::Init()` → 按 GraphicsBackend 初始化输入。
3. `RenderContext::Init()` → `RendererAPI::Init()` → 创建 OpenGLRendererAPI。
4. `RenderContext::SetCurrentWindow(m_Window.get())` → 供后续 SwapBuffers 使用。
5. `Renderer::Init()` → `Renderer2D::Init()`（SceneData、默认 2D Shader、四边形 VAO/VBO/IBO）→ `Renderer3D::Init()`（SceneData、默认 3D Shader）。
6. 创建 `RenderQueue`（Application 持有）。
7. `ImGuiLayer(GetGraphicsBackend())` 并压入层栈。

### 5.2 每帧渲染流程（Application::Run 主循环）

1. **准备帧缓冲与视口**  
   `RenderContext::GetAPI().SetViewport(...)` → `BeginRenderPass(nullptr)` → `SetClearColor` → `Clear(ClearColor|ClearDepth)`。

2. **逻辑更新**  
   遍历 LayerStack，`layer->OnUpdate(m_TimeStep)`。

3. **收集绘制命令**  
   `m_RenderQueue->Clear()`；遍历 Layer，若为 `IDrawable` 则 `SetCurrentLayerIndex` 后 `SubmitTo(*m_RenderQueue)`（Layer 内部可提交 2D Quad、3D Mesh 等）。

4. **排序**  
   `m_RenderQueue->Sort()`：2D/3D 分别按不透明/透明排序（层、相机、材质键、深度）。

5. **全局状态**  
   `SetDepthTest(true)`、`SetCullFace(false, true)`（可由后续按需覆盖）。

6. **按相机统一提交**  
   `m_RenderQueue->FlushAll()`：  
   - 收集本帧所有出现过的 `ViewCamera`；  
   - 对每个相机：若有 3D 命令则 `Renderer3D::BeginScene(cam)` → 不透明/透明分别 `Renderer3D::Submit(...)` → `Renderer3D::EndScene()`；若有 2D 命令则 `Renderer2D::BeginScene(cam)` → 不透明/透明分别 `Renderer2D::DrawQuad(...)` → `Renderer2D::EndScene()`。

7. **ImGui**  
   `m_ImGuiLayer->Begin()` → 各层 `OnImGuiRender()` → `m_ImGuiLayer->End()`。

8. **呈现**  
   `m_Window->OnUpdate()`（通常含交换缓冲）；或由 RenderContext 在别处调用 `SwapBuffers()`。

### 5.3 数据流小结

- **Layer**：实现 IDrawable，在 SubmitTo 中根据场景/逻辑向 RenderQueue 提交 DrawCommand2D / DrawCommand3D（含 position、size、color、VAO、transform、sortKey、ViewCamera 等）。
- **RenderQueue**：保管本帧所有 2D/3D 命令及排序后的下标；FlushAll 只读这些命令，按相机分批调用 Renderer2D/Renderer3D。
- **Renderer2D / Renderer3D**：保管当前场景的 ViewProjection 与默认 Shader；每帧由 FlushAll 先 BeginScene(cam) 写入 ViewProjection，再多次 DrawQuad/Submit 使用同一相机与 Shader，最后 EndScene。
- **RendererAPI**：不保管场景数据，只执行 Clear、DrawIndexed、状态与 RenderPass；具体 GL 状态与 FBO 由 OpenGL 实现持有。

#### 数据如何流向底层、变为画面

从“逻辑侧的一次绘制请求”到“屏幕像素”的完整链路如下。

1. **命令下沉到渲染器**  
   FlushAll 按相机、按排序顺序，对每条命令调用 `Renderer2D::DrawQuad(...)` 或 `Renderer3D::Submit(VAO, count, transform, color)`。此时数据仍是“场景空间”的：位置、尺寸、变换矩阵、颜色、以及要用的几何（2D 用内置四边形 VAO，3D 用命令里带的 VAO）。

2. **着色器与 uniform 写入**  
   Renderer2D/3D 在 BeginScene 时已绑定默认 Shader 并上传 `u_ViewProjection`；每次 DrawQuad/Submit 时再上传本图元的 `u_Transform`、`u_Color`。底层（如 OpenGLShader）把这些 uniform 通过 `glUniform*` 写入当前 GL program，供管线中顶点/片段着色器使用。

3. **几何绑定与 DrawIndexed**  
   Renderer2D/3D 在每次绘制前绑定对应的 VertexArray（2D 为四边形 VAO，3D 为命令中的 VAO），然后调用 `RendererAPI::Get().DrawIndexed(vertexArray, indexCount)`。平台层只做抽象调用；底层（如 OpenGLRendererAPI）根据当前绑定的 VAO 得到 VBO/EBO，最终发出 `glDrawElements(GL_TRIANGLES, count, ...)`。此时：
   - **顶点数据**（位置、颜色等）已在 Init 或资源创建时上传到 GPU（VBO），VAO 记录布局；
   - **索引**在 EBO 中，DrawIndexed 的 count 决定从 EBO 里读多少个索引、画多少个三角形。

4. **GPU 管线执行**  
   驱动收到 DrawCall 后，GPU 按顶点着色器 → 光栅化 → 片段着色器 执行：顶点着色器用 `u_ViewProjection`、`u_Transform` 与顶点属性算出裁剪空间坐标；光栅化生成像素片段；片段着色器用 `u_Color` 等写出最终颜色，写入当前绑定的**帧缓冲**（默认即窗口的后缓冲，或 BeginRenderPass 指定的 FBO）。

5. **帧缓冲与呈现**  
   整帧中所有 DrawCall 都画到同一帧缓冲（默认是窗口的 back buffer）。ImGui 绘制同样写入该缓冲。帧结束由 `Window::OnUpdate()` 或 `RenderContext::SwapBuffers()` 执行前后缓冲交换（如 `glfwSwapBuffers`），把刚画完的 back buffer 变为用户可见的前缓冲，下一帧的绘制目标变为新的 back buffer。  
   因此：**数据流** = Layer 提交命令 → RenderQueue 排序 → Renderer2D/3D 写 uniform、绑 VAO、调 DrawIndexed → RendererAPI/OpenGL 发 glDrawElements → GPU 执行管线写帧缓冲 → 交换缓冲后变为画面。

---

## 六、小结表

| 层次 | 保管的数据 | 工作流角色 |
|------|------------|------------|
| **平台无关** | Renderer2D/3D 的 SceneData、默认 Shader、2D 四边形 VAO；RenderQueue 的命令与排序结果 | 提交接口、排序、按相机 Flush |
| **平台兼容** | RenderContext 的当前窗口；RendererAPI 单例指针；资源无全局状态 | 抽象 API、资源工厂、Init/Shutdown/SwapBuffers |
| **底层实现** | 各 OpenGL* 对象的 GL 句柄（program、VAO、VBO、EBO、FBO、Texture 等） | 执行清屏、绘制、状态与 RenderPass |

新增图形后端时：在 `Platform/Backend/GraphicsBackend` 增加枚举与分支；在 `RendererAPI::Init`、各资源 `Create*`、Pipeline、Framebuffer、Window、Input、ImGui 的工厂中增加分支；在 `Backends/` 下新增目录并实现对应接口，保持 Renderer 与 RenderQueue 代码不变。
