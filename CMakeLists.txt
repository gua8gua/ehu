cmake_minimum_required(VERSION 3.15)
project(Ehu)

# 生成 compile_commands.json，供 VS Code / Cursor 的 C++ 扩展做 Ctrl+点击 跳转
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置构建类型
set(CMAKE_CONFIGURATION_TYPES Debug Release)

set(BUILD_DIR "${CMAKE_BINARY_DIR}")
set(CMAKE_SKIP_BUILD_RPATH TRUE)

# 添加子项目
add_subdirectory(Ehu)
add_subdirectory(SandBox)
add_subdirectory(EhuEditor)

set(CMAKE_INSTALL_PREFIX "install")
