# Hazelnut 源码 (src)

编辑器 C++ 源码：应用入口与编辑器主层、面板实现。

## 结构

| 文件/路径 | 说明 |
|-----------|------|
| **HazelnutApp.cpp** | 入口：创建 Application，PushLayer(EditorLayer) |
| **EditorLayer.h/cpp** | 编辑器主层：视口 Framebuffer、场景编辑/运行/模拟、项目与场景的打开/保存、工具栏、ImGui 面板布局 |
| [Panels](./Panels/README.md) | 场景层级面板、内容浏览器面板等 |
