# scripts 脚本目录

环境与工程生成脚本：依赖拉取、Vulkan 配置、Premake 生成等。

## 主要脚本

| 脚本 | 功能 |
|------|------|
| **Setup.bat** | 主入口：检查/安装 Python、拉取依赖、执行 Vulkan 安装或调试库下载，最后调用 Win-GenProjects.bat 生成 VS 工程 |
| **Setup.py** | 总控 Python 脚本：协调各子步骤 |
| **SetupPython.py** | 检查或安装 Python 环境 |
| **SetupPremake.py** | 检查或下载 Premake 等构建工具 |
| **SetupVulkan.py** | 检查 Vulkan SDK、下载 Vulkan SDK 安装包或调试库 |
| **Utils.py** | 公共工具函数（下载、解压、路径等） |
| **Win-GenProjects.bat** | 调用 Premake 生成 Visual Studio 解决方案与项目文件 |

## 使用方式

1. 首次：在 `scripts` 目录下运行 `Setup.bat`，完成依赖与 Vulkan 配置后会自动生成工程。
2. 仅重新生成工程：运行 `Win-GenProjects.bat`。

## 架构要点

- 依赖与 SDK 路径通过环境变量或 Dependencies.lua 与 premake 联动，保证生成的解决方案能正确链接 Vulkan、Mono 等。
