# Hazelnut 编辑器

基于 Hazel 的桌面编辑器应用，用于设计场景、管理资源、运行与调试游戏。

## 目录结构

| 路径 | 说明 |
|------|------|
| [src](./src/README.md) | C++ 源码：入口、EditorLayer、各类面板 |
| assets | 编辑器用资源：着色器、字体等 |
| SandboxProject | 示例项目：.hproj、场景、脚本等 |
| mono | Mono 运行时相关配置（如 .NET 4.x 工具链） |
| premake5.lua | 编辑器项目定义 |
| imgui.ini | ImGui 布局配置 |

## 功能概览

- **入口**：HazelnutApp 创建 Application 并压入 EditorLayer。
- **EditorLayer**：主编辑器层：视口、场景层级、内容浏览器、工具栏、项目/场景的新建/打开/保存、运行/模拟/暂停、实体复制等。
- **面板**：SceneHierarchyPanel（场景层级与实体选择）、ContentBrowserPanel（资产浏览与操作）。

## 架构要点

- 启动项目为 premake 中的 startproject，依赖 Hazel 与 Hazel-ScriptCore；C# 脚本输出目录指向 Hazelnut 的 Resources/Scripts，由编辑器加载。
