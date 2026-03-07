# Project 模块

项目配置与路径管理：当前活动项目、资产目录、脚本模块路径等。

## 功能概览

| 组件 | 功能 |
|------|------|
| **Project** | 项目单例：GetProjectDirectory、GetAssetDirectory、GetAssetFileSystemPath；New/Load/SaveActive |
| **ProjectConfig** | 项目配置：Name、StartScene、AssetDirectory、ScriptModulePath |
| **ProjectSerializer** | 项目序列化/反序列化（如 .hproj 文件） |

## 架构要点

- 活动项目由 `Project::s_ActiveProject` 维护，编辑器打开项目时 Load，关闭时清空。
- 资产路径均相对于项目目录与 AssetDirectory，便于跨机器迁移与资源引用。
