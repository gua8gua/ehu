# Hazel/vendor 第三方库

引擎核心所依赖的第三方库与子模块，由 Hazel/premake5.lua 与根目录 Dependencies.lua 引用。

## 主要依赖

| 库 | 用途 |
|------|------|
| **Box2D** | 2D 物理引擎，Physics2D 模块封装 |
| **GLFW** | 跨平台窗口与输入，Windows 窗口实现使用 |
| **Glad** | OpenGL 加载器，OpenGL 后端使用 |
| **ImGui** | 即时模式 UI，ImGui 模块集成 |
| **ImGuizmo** | 编辑器 Gizmo（平移/旋转/缩放等） |
| **yaml-cpp** | YAML 解析，场景/项目序列化 |
| **entt** | ECS 库，Scene/Entity 底层 |
| **glm** | 数学库，渲染与变换 |
| **stb_image** | 图像加载，纹理导入 |
| **filewatch** | 文件监视，资源热重载等 |
| **msdf-atlas-gen / msdfgen** | MSDF 字体图集生成，文本渲染 |
| **mono** | C# 运行时，Scripting 模块使用 |
| **spdlog** | 日志（常为 git 子模块） |
| **Vulkan/shaderc/SPIRV-Cross** | 通过 VULKAN_SDK 引用，用于 Shader 编译或未来 Vulkan 后端 |

## 架构要点

- 头文件与库路径在 Dependencies.lua 中统一配置，Hazel 的 premake5.lua 通过 `include "vendor/..."` 将各库加入工程。
- 引擎代码仅依赖上述库的公开 API，平台相关库（如 Vulkan）通过环境变量定位。
