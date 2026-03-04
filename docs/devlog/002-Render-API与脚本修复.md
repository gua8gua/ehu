# 开发日志 #002 — Render API 补齐、3D 示例与脚本修复

**日期**：2025-03-04

---

## 一、本阶段概要

在开发日志 #001 的基础上，完成 **Render API 层** 剩余模块（RenderContext、Pipeline、Texture2D、Framebuffer、States、RenderPass），在 **SandBox** 中增加 **3D 透视示例**，并修复 **scripts** 下 Ninja 构建脚本在 VS 环境下的失效问题。

---

## 二、Render API 层补充

### 2.1 已实现模块（本阶段）

| 模块 | 说明 |
|------|------|
| **Texture2D** | Platform 抽象（Create/CreateFromFile/Bind/SetData），OpenGL 实现；从文件加载需 `EHU_USE_STB_IMAGE` + stb_image.h |
| **Framebuffer** | Platform 抽象（FramebufferSpec、Bind/Unbind/Resize、GetColorAttachmentRendererID），OpenGL 实现，支持 MRT、深度/模板附件，API 兼容 OpenGL 3.3 |
| **States** | RendererAPI：SetDepthTest、SetBlend、SetCullFace、SetViewport；Clear(clearFlags) 支持 color/depth/stencil 位掩码 |
| **RenderPass** | RendererAPI::BeginRenderPass(Framebuffer*)、EndRenderPass()，用于绑定 FBO 或解绑回默认帧缓冲 |
| **RenderContext** | Init/Shutdown、GetAPI()、SetCurrentWindow(Window*)、SwapBuffers()；Application 通过 RenderContext 初始化与 Present |
| **Pipeline** | Shader + VertexArray + 状态（深度/混合/背面剔除），Bind() + DrawIndexed()；Platform 抽象 + OpenGL 实现 |
| **RendererAPI** | 新增 Shutdown()；Clear 改为 Clear(uint32_t clearFlags)，枚举 ClearColor/ClearDepth/ClearStencil |

### 2.2 接口与路径约定

- 接口统一放在 **Platform/**（RendererAPI、RenderContext、Pipeline、Texture2D、Framebuffer 等）。
- 实现放在 **Backends/OpenGL_GLFW/**（OpenGL* 对应实现）。
- OpenGL 实现采用 **3.3 兼容 API**（如 glGenTextures/glBindTexture、glGenFramebuffers 等），避免依赖 4.5 专有接口。

### 2.3 主循环与清屏

- Application 主循环改为通过 **RenderContext::GetAPI()** 做 SetClearColor / Clear。
- Clear 使用 **ClearColor | ClearDepth**，以便 3D 深度测试正确。

---

## 三、Renderer 层与 3D 示例

### 3.1 Renderer2D 扩展

- 新增 **BeginScene(const Camera& camera)** 重载，使用任意相机的 GetViewProjectionMatrix()，从而支持 **PerspectiveCamera** 做简单 3D 透视绘制。

### 3.2 SandBox 3D 示例

- 新增 **Example3DLayer**：使用 **PerspectiveCamera**(45°, 1280/720, 0.1, 100)，相机位于 (0, 0, 6)。
- 每帧开启深度测试后，绘制 5 个不同 z 的彩色四边形（前排 z=0 红/绿，中排 z=-2 蓝/黄，后排 z=-3.5 紫），用于展示透视与深度遮挡。
- 当前 SandBox 仅推送 Example3DLayer，便于直接观察 3D 效果。

---

## 四、脚本修复（scripts/）

### 4.1 问题原因

- **gen_compile_commands.ps1**、**build_ninja.ps1** 原先通过 **Launch-VsDevShell.ps1** 加载 VS 环境。
- Launch-VsDevShell 会启动**新的开发壳**，不会修改**当前 PowerShell 进程**的环境。
- 因此脚本内后续的 `cmake` / `ninja` 仍在无 VS 环境（无 cl、标准库路径等）的进程中执行，导致编译报错（如无法打开 `<vector>`、`<string>` 等）。

### 4.2 修改方案

- 改为使用 **vcvarsall.bat** 在**同一进程**内注入环境后执行命令：
  - **gen_compile_commands.ps1**：`cmd /c "call vcvarsall.bat amd64 && cd /d build-ninja && cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug <repo_root>"`
  - **build_ninja.ps1**：`cmd /c "call vcvarsall.bat amd64 && cd /d build-ninja && ninja"`
- 在多个可能路径下查找 vcvarsall.bat（含自定义 `D:\ProgramFiles\...` 与 `%ProgramFiles%\...`），便于不同安装位置使用。

### 4.3 使用方式

```powershell
.\scripts\gen_compile_commands.ps1   # 首次或重新配置
.\scripts\build_ninja.ps1            # 编译
```

---

## 五、其他修改

### 5.1 CMake

- **Ehu/CMakeLists.txt**：Debug 下仅对**非 MSVC** 添加 `-g`，避免 MSVC 报“未知选项 -g”。
- **SandBox/CMakeLists.txt**：同上，MSVC 下不添加 `-g`。

### 5.2 头文件与依赖

- **RenderContext.h**：包含 **RendererAPI.h**，使 SandBox 等调用 GetAPI() 时获得完整类型。
- **RendererAPI.h**：`#include "Core.h"` 改为 **"Core/Core.h"**，保证在 SandBox 的 include 路径下能正确解析。

### 5.3 文档

- **TODO.md**：Render API 表中 Render Context、Pipelines、Swap chain 等已做项已更新为「已做」并补充说明。

---

## 六、当前构建与运行

- **配置**：`.\scripts\gen_compile_commands.ps1` 生成 build-ninja 与 compile_commands.json。
- **编译**：`.\scripts\build_ninja.ps1` 可完成 EhuLib + SandBox 的 Ninja 构建。
- **运行**：`build-ninja\bin\Debug\SandBox.exe` 运行后可见透视下的多层彩色四边形与深度遮挡。

---

## 七、文档索引（更新）

| 文件 | 内容 |
|------|------|
| [README.md](../README.md) | 项目说明、结构、构建、架构与使用 |
| [ARCHITECTURE.md](../ARCHITECTURE.md) | 模块依赖、分层、事件流、扩展建议 |
| [TODO.md](../TODO.md) | 待办清单与建议完成顺序 |
| [RENDERER_ARCHITECTURE.md](../RENDERER_ARCHITECTURE.md) | 渲染调度与对外接口 |
| [devlog/001-项目当前进度.md](001-项目当前进度.md) | 开发日志 #001 |
| [devlog/002-Render-API与脚本修复.md](002-Render-API与脚本修复.md) | 本日志 |
