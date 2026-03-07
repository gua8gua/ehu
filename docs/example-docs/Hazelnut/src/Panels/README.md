# Panels 面板

编辑器中以 ImGui 实现的各类功能面板。

## 面板列表

| 面板 | 功能 |
|------|------|
| **SceneHierarchyPanel** | 场景层级：列出当前场景中所有实体、选中高亮、右键菜单（创建/删除/复制实体等）、与 Inspector 联动 |
| **ContentBrowserPanel** | 内容浏览器：显示项目 Asset 目录、文件/文件夹导航、资源图标、双击打开（如场景）、与资源导入/管理配合 |

## 架构要点

- 各面板在 EditorLayer::OnImGuiRender 中调用，通过 Ref<Scene> 与 Project 路径访问当前场景与资产。
- 选中实体、当前资源路径等状态由 EditorLayer 或共享状态管理，供多面板联动。
