# Hazel 引擎核心

Hazel 是引擎的静态库核心，被 Hazelnut（编辑器）和 Sandbox（示例应用）链接。提供窗口、输入、渲染、场景、脚本桥接、项目/序列化、ImGui、物理等能力。

## 目录结构


| 路径                     | 说明                                                                                             |
| ---------------------- | ---------------------------------------------------------------------------------------------- |
| [src](./src/README.md) | 引擎源码根：预编译头、对外头文件、Hazel 模块与 Platform 实现                                                         |
| vendor                 | 第三方库（Box2D、GLFW、Glad、ImGui、yaml-cpp、Mono、spdlog、entt、glm、stb_image、filewatch、msdf-atlas-gen 等） |
| premake5.lua           | 静态库项目定义                                                                                        |


## 对外接口

应用只需包含 `src/Hazel.h`，即可使用 Core、Events、Renderer、Scene、Project、ImGui 等对外 API。

## 产出

- 静态库（如 `Hazel.lib`），供 Hazelnut、Sandbox 链接。

