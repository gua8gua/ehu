# Ehu Engine

Ehu 是一个基于 C++ 的游戏引擎雏形（跟随 [The Cherno](https://www.youtube.com/c/TheCherno) 系列），采用分层架构与事件驱动，**核心与底层分离**：Core/Platform/Backends 结构，当前后端为 OpenGL + GLFW，支持 Windows。

- **构建**：默认使用 **scripts** 下的编译脚本。在项目根目录执行：  
  **配置** `.\scripts\gen_compile_commands.ps1` → **编译** `.\scripts\build_ninja.ps1`  
  输出目录：`build-ninja/`，可执行文件：`build-ninja/bin/Debug/SandBox.exe`。  
  其他方式见 [docs/README.md](docs/README.md)。
- **文档**：见 **[docs/](docs/)**  
  - [docs/README.md](docs/README.md) — 项目说明、目录结构、构建、架构与使用  
  - [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — 分层、模块依赖、后端选择与扩展  
  - [docs/TODO.md](docs/TODO.md) — 待办（Timestep、渲染管线、相机等）
