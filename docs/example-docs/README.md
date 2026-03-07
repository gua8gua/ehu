# Hazel 引擎文档总览

本目录按项目原有文件夹层级组织，对各模块的功能与架构进行说明。Hazel 是一个面向 Windows 的早期交互式应用与渲染引擎，同时作为游戏引擎设计与架构的教学项目。

## 文档结构（与代码目录对应）

| 路径 | 说明 |
|------|------|
| [Hazel](./Hazel/README.md) | 引擎核心静态库：窗口、输入、渲染、场景、脚本、项目、ImGui、物理等 |
| [Hazel-ScriptCore](./Hazel-ScriptCore/README.md) | C# 脚本 API，供游戏脚本使用的类型与内部调用 |
| [Hazelnut](./Hazelnut/README.md) | 基于 Hazel 的编辑器应用 |
| [Sandbox](./Sandbox/README.md) | 示例/测试游戏应用 |
| [scripts](./scripts/README.md) | 环境与工程生成脚本（Setup、Premake、Vulkan 等） |
| [vendor](./vendor/README.md) | 根级构建工具（Premake） |

## 构建与依赖

- **工作区入口**：根目录 `premake5.lua`，定义 Debug/Release/Dist，包含各子项目。
- **依赖配置**：`Dependencies.lua` 指定第三方头文件与库路径（Vulkan、Mono、Box2D、GLFW、Glad、ImGui、yaml-cpp 等）。
- **生成工程**：运行 `scripts/Setup.bat` 配置环境后，使用 `scripts/Win-GenProjects.bat` 生成 Visual Studio 解决方案。

## 项目分组（premake）

- **Dependencies**：Box2D、GLFW、Glad、msdf-atlas-gen、imgui、yaml-cpp、premake 等。
- **Core**：Hazel（引擎）、Hazel-ScriptCore（C# API）。
- **Tools**：Hazelnut（编辑器）。
- **Misc**：Sandbox（示例应用）。

## 相关链接

- 项目 README：[../README.md](../README.md)
- 贡献指南：`.github/CONTRIBUTING.md`
