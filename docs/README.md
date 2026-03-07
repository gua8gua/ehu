# Ehu 引擎 — 项目说明

Ehu 是一个基于 C++ 的游戏引擎雏形（跟随 Cherno 系列教程），采用分层架构与事件驱动，**底层与核心分离**：核心层不依赖具体窗口/渲染 API，平台与后端可切换。当前支持 Windows，默认后端为 OpenGL + GLFW，ImGui 用于调试界面。

---

## 项目结构

```
ehu/
├── CMakeLists.txt              # 根 CMake
├── Ehu/                        # 引擎库 EhuLib
│   ├── CMakeLists.txt
│   └── src/
│       ├── CMakeLists.txt
│       └── Ehu/
│           ├── CmakeLists.txt        # 库根：ehupch + 子目录
│           ├── ehupch.h / ehupch.cpp
│           ├── Ehu.h                  # 对外统一头文件
│           ├── EntryPoint.h           # main() 与启动流程
│           ├── Core/                  # 核心层（无平台/渲染依赖）
│           │   ├── Application, Layer, LayerStack, Log
│           │   ├── Core.h, KeyCodes.h, MouseCodes.h
│           │   └── CMakeLists.txt
│           ├── Events/                # 事件类型与分发
│           │   ├── Event, KeyEvent, MouseEvent, ApplicationEvent
│           │   └── CMakeLists.txt
│           ├── Platform/              # 平台抽象：后端选择与接口
│           │   ├── GraphicsBackend, Window, Input, RendererAPI
│           │   └── CMakeLists.txt
│           ├── Scene/                 # 场景模块（纯数据）：Scene, SceneNode, SceneEntity
│           │   └── CMakeLists.txt
│           ├── Renderer/              # 渲染器层（平台无关）
│           │   ├── Camera/            # OrthographicCamera, PerspectiveCamera
│           │   ├── Material/          # Material, MaterialProperties
│           │   ├── SceneLayer, Renderer2D, Renderer3D, Renderer, RendererModule.h
│           │   └── CMakeLists.txt
│           ├── ImGui/                 # ImGui 集成（仅依赖抽象）
│           │   ├── ImGuiLayer, ImGuiBackend（接口 + 工厂）
│           │   ├── ImGuiBuild.cpp
│           │   └── CMakeLists.txt
│           └── Backends/              # 具体后端实现
│               ├── CMakeLists.txt
│               └── OpenGL_GLFW/
│                   ├── WindowsWindow, WindowsInput
│                   ├── ImGuiBackendGLFWOpenGL, OpenGLRendererAPI
│                   └── CMakeLists.txt
│   └── vendor/                 # 第三方：GLFW, GLAD, imgui, glm, spdlog
├── SandBox/                    # 示例可执行程序
│   ├── CMakeLists.txt
│   └── src/SandApp.cpp         # CreateApplication() + 自定义 Layer
└── docs/
    ├── README.md               # 本文件
    ├── ARCHITECTURE.md          # 模块依赖、分层、扩展
    ├── TODO.md                  # 待办清单
    └── devlog/                  # 开发日志
        └── 001-项目当前进度.md
```

- **Core**：Application、Layer、LayerStack、Log、键鼠码等，不包含任何 GLFW/OpenGL 头文件。
- **Platform**：GraphicsBackend、Window/Input/RendererAPI 的抽象接口与工厂（`Create`/`Init` 按后端分发）。
- **Backends/OpenGL_GLFW**：当前唯一后端，实现窗口、输入、ImGui 绑定与清屏（OpenGLRendererAPI）。

---

## 构建

### 环境要求

- CMake 3.15+
- C++17 编译器（MSVC / MinGW / Clang）
- Windows（当前仅支持该平台）

### 方式一：Visual Studio 生成器（默认）

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Debug
```

可执行文件：`build/bin/Debug/SandBox.exe`（库在 `build/bin-int/Debug/`）。

### 方式二：Ninja（生成 compile_commands.json，供 C++ 插件用）

**配置与构建均需在 VS 开发者环境中执行**（否则会报 `rc`/`kernel32.lib` 找不到）。

1. **配置**（生成 `build-ninja/compile_commands.json` 与构建文件）：
  - 在 **PowerShell** 中（建议在项目根目录）：
  - 在 **cmd** 中（项目根目录）：
    ```cmd
    powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\gen_compile_commands.ps1
    ```
2. **编译**：
  ```powershell
  powershell -NoProfile -ExecutionPolicy  Bypass -File  .\scripts\build_ninja.ps1
  ```

输出位置（与 CMake 中 `BUILD_DIR` 一致）：

- 可执行文件：`build-ninja/bin/Debug/SandBox.exe`
- 静态库：`build-ninja/bin-int/Debug/EhuLib.lib`
- 编译数据库：`build-ninja/compile_commands.json`（给 VS Code/Cursor 的 C++ 扩展用）

**断言**：Debug 配置下 CMake 已定义 `EHU_ENABLE_ASSERTS`，`EHU_ASSERT` / `EHU_CORE_ASSERT` 生效；Release/Dist 不定义，断言为空操作。

若未使用脚本，可先打开 **“Developer PowerShell for VS 2022”**，再执行：
`cd build-ninja` → `cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug ..` → `ninja`。

---

## 架构概览

### 应用与主循环

- **Application**（Core）：单例，持有 `Window`、`LayerStack`、`ImGuiLayer`。
- **Run()** 每帧：
  1. `RendererAPI::Get().SetClearColor()` / `Clear()` 清屏（由后端实现，如 OpenGL）
  2. 遍历所有 Layer 调用 `OnUpdate()`
  3. ImGui `Begin` → 各 Layer `OnImGuiRender()` → ImGui `End`
  4. `Window::OnUpdate()`（事件轮询与交换缓冲）

用户通过实现 `CreateApplication()` 返回自定义 `Application` 子类。

### 层与层栈、Scene 与 Layer

- **Layer**：可重写 `OnAttach`、`OnDetach`、`OnUpdate`、`OnImGuiRender`、`OnEvent`。负责“怎么做（逻辑与渲染顺序）”。
- **Scene**：独立模块，纯数据；管理 SceneEntity，实体由场景拥有。渲染由引擎提供的 **SceneLayer** 负责提交，用户只创建场景、设置实体状态（如 SetPosition/SetRotation）。详见 [ARCHITECTURE.md](ARCHITECTURE.md) 与 [RENDERER_ARCHITECTURE.md](RENDERER_ARCHITECTURE.md)。
- **LayerStack**：普通层与 Overlay；事件从栈顶向栈底派发，可被 `Handled` 截断。

### 事件系统

- **Event**、**EventDispatcher**：在 `Events/`；窗口实现（如 Backends 中的 WindowsWindow）将底层回调转为事件并调用 `EventCallback`。

### 窗口、输入与渲染抽象

- **Window**：`Platform/IO/Window.h` 抽象，`Window::Create(WindowProps)` 在 `Platform/IO/Window.cpp` 中按 `GetGraphicsBackend()` 分发。
- **Input**：`Platform/IO/Input.h` 抽象，`Input::Init()` 按后端创建实现，由 Application 在窗口创建后调用。
- **RendererAPI**：`Platform/Render/RendererAPI.h` 抽象（`SetClearColor`、`Clear`），`RendererAPI::Init()` 按后端创建；Application 主循环只调用 `RendererAPI::Get().Clear()`，无直接 OpenGL 依赖。
- **ImGuiBackend**：`ImGui/ImGuiBackend.h` 抽象，`ImGuiBackend::Create(backend)` 返回对应实现（如 `Backends/OpenGL_GLFW/ImGuiBackendGLFWOpenGL`）。

### 图形/窗口后端选择

- **GraphicsBackend** 与 **GetGraphicsBackend()** 位于 `Platform/Backend/`，可读环境变量 **EHU_GRAPHICS_BACKEND**（如 `OpenGL_GLFW`），默认 `OpenGL_GLFW`。
- 新增后端时：在 `GetGraphicsBackend()`、`Window::Create`、`Input::Init`、`RendererAPI::Init`、`ImGuiBackend::Create` 中增加分支，并在 `Backends/` 下添加新实现目录。

### 日志与宏

- **Log::Init()**：在 `EntryPoint` 中调用；宏 `EHU_CORE_`* / `EHU_`*。
- **Core/Core.h**：`EHU_API`、`EHU_ASSERT` / `EHU_CORE_ASSERT`、`BIT(x)`；预定义 `EHU_PLATFORM_WINDOWS`、`EHU_DEBUG` / `EHU_RELEASE` / `EHU_DIST` 等。

---

## 如何编写一个“游戏”应用

1. 工程链接 **EhuLib**，实现 `Ehu::Application* Ehu::CreateApplication()`。
2. 继承 `Ehu::Application`，在构造函数中 `PushLayer(...)` / `PushOverlay(...)`。
3. 继承 `Ehu::Layer` 实现 `OnUpdate(const TimeStep&)`、`OnImGuiRender`、`OnEvent`。
4. 包含 **Ehu.h**（推荐）或按需包含 `Core/Application.h`、`Core/Layer.h`、`Platform/IO/Input.h` 等。

参考 **SandBox** 中的 `SandApp` 与 `ExampleLayer`。SandBox 的 include 路径需包含 `Ehu/src` 与 `Ehu/src/Ehu`（见 SandBox/CMakeLists.txt）。

---

## 依赖（vendor）


| 依赖     | 用途                   |
| ------ | -------------------- |
| GLFW   | 窗口与输入（仅 Backends 使用） |
| GLAD   | OpenGL 加载            |
| imgui  | 即时模式 GUI             |
| glm    | 数学                   |
| spdlog | 日志                   |


---

## 使用 glm 做数学运算

项目已集成 [GLM](https://github.com/g-truc/glm)（OpenGL Mathematics），用于向量、矩阵、四元数等运算，无需额外配置。

### 操作步骤

1. **确认已集成（已完成）**
  - 源码在 `Ehu/vendor/glm`。  
  - `Ehu/CMakeLists.txt` 中已有：
    - `add_subdirectory(vendor/glm)`
    - `target_link_libraries(EhuLib PUBLIC glm)`
  - EhuLib 及链接了 EhuLib 的目标（如 SandBox）会自动获得 glm 的 include 与链接。
2. **在引擎或 SandBox 代码中使用**
  - 在需要数学运算的 `.cpp` 或 `.h` 中按需包含，例如：
  - 使用 `glm::` 命名空间下的类型与函数，例如：
    ```cpp
    glm::vec3 position(1.0f, 0.0f, 0.0f);
    glm::mat4 view = glm::lookAt(eye, center, up);
    glm::mat4 proj = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f, -1.0f, 1.0f);
    ```
3. **若将来要更换 glm 版本或路径**
  - 修改 `Ehu/CMakeLists.txt` 中的 `add_subdirectory(vendor/glm)` 指向新路径，或通过 `find_package(glm)` 使用系统/安装的 glm，并保持 `target_link_libraries(EhuLib PUBLIC glm)`（或 `glm::glm`）。

### 常用头文件速查


| 头文件                            | 用途                                                      |
| ------------------------------ | ------------------------------------------------------- |
| `glm/glm.hpp`                  | 向量、矩阵基础类型                                               |
| `glm/gtc/matrix_transform.hpp` | 变换：translate, rotate, scale, ortho, perspective, lookAt |
| `glm/gtc/type_ptr.hpp`         | value_ptr（传矩阵/向量给 OpenGL）                               |
| `glm/gtc/quaternion.hpp`       | 四元数                                                     |


---

## 文档索引（本目录）


| 文件                                                   | 内容                                     |
| ---------------------------------------------------- | -------------------------------------- |
| [README.md](README.md)                               | 项目说明、结构、构建、架构与使用。                      |
| [ARCHITECTURE.md](ARCHITECTURE.md)                   | 模块依赖、分层、事件流、后端选择与扩展。                   |
| [RENDERER_ARCHITECTURE.md](RENDERER_ARCHITECTURE.md) | 渲染架构调度关系、主循环、Renderer 与 Render API 分层。 |
| [TODO.md](TODO.md)                                   | 待办：Timestep、渲染管线、相机、命名与层级等。            |
| [devlog/](devlog/)                                   | 开发日志，记录项目进度与变更。                        |


