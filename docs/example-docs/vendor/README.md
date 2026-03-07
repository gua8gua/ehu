# vendor（根级）

根目录下的第三方/工具依赖，当前仅包含 Premake 相关，用于生成解决方案与项目文件。

## 结构


| 路径          | 说明                                                                                                         |
| ----------- | ---------------------------------------------------------------------------------------------------------- |
| **premake** | Premake 可执行或脚本；premake5.lua 定义 Utility 项目；premake_customization/solution_items.lua 将 .editorconfig 等加入解决方案 |


## 说明

- 引擎自身的第三方库（Box2D、GLFW、Glad、ImGui、yaml-cpp、Mono、spdlog 等）位于 `Hazel/vendor`，由根目录 premake5.lua 通过 `include "Hazel/vendor/..."` 引入。
- 本目录的 vendor 仅指“根级”构建用工具，与 `Hazel/vendor` 区分。

