# Hazel 源码根 (src)

引擎 C++ 源码的根目录，包含预编译头、对外统一头文件、按功能划分的引擎模块以及平台相关实现。

## 结构

| 路径 | 说明 |
|------|------|
| [Hazel](./Hazel/README.md) | 按功能划分的引擎模块（Core、Events、Renderer、Scene、Project、Scripting、ImGui、Math、Physics、Debug、Utils、UI） |
| [Platform](./Platform/README.md) | 平台相关实现（Windows、OpenGL） |
| Hazel.h | 对外统一头文件，应用只需包含此头 |
| hzpch.h / hzpch.cpp | 预编译头 |

## 架构要点

- **Hazel** 目录下为与平台无关的引擎逻辑与抽象接口。
- **Platform** 目录下为各平台/图形 API 的具体实现（如 Windows 窗口与输入、OpenGL 渲染管线）。
