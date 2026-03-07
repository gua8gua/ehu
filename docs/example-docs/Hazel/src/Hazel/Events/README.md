# Events 模块

事件系统：定义各类事件的类型与数据，供 Application 与 Layer 进行事件驱动更新。

## 功能概览

| 组件 | 功能 |
|------|------|
| **Event** | 事件基类：EventType、Category、是否已处理；`EventDispatcher` 模板按类型分发 |
| **ApplicationEvent** | 应用级事件：窗口关闭、窗口调整大小等 |
| **KeyEvent** | 键盘事件：KeyPressed、KeyReleased、KeyTyped |
| **MouseEvent** | 鼠标事件：移动、滚轮、按钮按下/释放等 |

## 架构要点

- 事件在 Application::OnEvent 中先传给 ImGui，再自顶向下传给 LayerStack 中的各 Layer。
- 使用 `EventDispatcher` 与 `BIND_EVENT_FN` 将事件绑定到成员函数，按事件类型调用对应处理函数。
