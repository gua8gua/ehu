# ImGui 模块

ImGui 集成层：将 ImGui 作为一层 Overlay 接入引擎，处理输入与渲染。

## 功能概览

| 组件 | 功能 |
|------|------|
| **ImGuiLayer** | ImGui 层：OnAttach/OnDetach 中初始化/关闭 ImGui；每帧 Begin/End、处理输入与字体；Application 持有单例并可通过 GetImGuiLayer() 访问 |
| **ImGuiBuild** | ImGui 构建相关配置或扩展 |

## 架构要点

- ImGuiLayer 通常作为最顶层 Overlay 压入 LayerStack，事件先传给 ImGui 消费，再传给下层 Layer。
- 编辑器（Hazelnut）与示例（Sandbox）的 UI 均通过 ImGui 绘制。
