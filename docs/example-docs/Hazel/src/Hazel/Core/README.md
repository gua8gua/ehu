# Core 模块

引擎核心基础层：应用生命周期、层级管理、日志、窗口、输入、时间与工具类型。

## 功能概览

| 组件 | 功能 |
|------|------|
| **Application** | 应用主类：创建窗口、维护 LayerStack、事件分发、主线程任务队列、ImGui 层；单例 `Application::Get()` |
| **Layer / LayerStack** | 层级：Layer 可接收 OnUpdate/OnEvent/OnImGuiRender；LayerStack 管理 Layer 与 Overlay 的压栈顺序 |
| **Log** | 日志接口（基于 spdlog），区分 Core 与 Client 日志 |
| **Window** | 窗口抽象接口，由 Platform（如 WindowsWindow）实现 |
| **Input** | 键盘/鼠标输入查询（键/按钮状态），平台实现（如 WindowsInput） |
| **KeyCodes / MouseCodes** | 键码与鼠标按钮码定义 |
| **Timestep** | 帧时间步长，用于与时间相关的更新 |
| **Timer** | 计时器工具 |
| **UUID** | 全局唯一 ID，用于实体、资源等 |
| **FileSystem** | 文件系统路径与基础操作 |
| **Assert** | 断言宏（HZ_CORE_ASSERT 等） |
| **Base** | 基础类型与宏（Ref、Scope、HZ_* 等） |
| **Buffer** | 通用内存缓冲区抽象 |
| **EntryPoint** | 应用入口宏，定义 `main` 并启动 Application |
| **PlatformDetection** | 平台/编译器检测宏 |

## 架构要点

- Application 持有 Window、LayerStack、ImGuiLayer，在 Run() 中驱动主循环并分发事件。
- 所有 Hazel 应用通过 EntryPoint 宏提供统一入口，由 Application 子类指定具体行为。
