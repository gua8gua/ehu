# Windows 平台实现

Windows 下的窗口、输入与平台工具实现。

## 功能概览

| 组件 | 功能 |
|------|------|
| **WindowsWindow** | Window 接口实现：GLFW 创建窗口、与 GraphicsContext（OpenGL）绑定、事件回调转发、VSync 等 |
| **WindowsInput** | Input 接口实现：键盘/鼠标状态查询（基于 GLFW 或 Win32 API） |
| **WindowsPlatformUtils** | PlatformUtils 实现：剪贴板、对话框等 Win32 调用 |

## 架构要点

- 窗口创建与消息循环由 GLFW 处理，Hazel 主循环在 Application::Run 中调用 glfwPollEvents 与各 Layer 的 OnUpdate。
