cmake_minimum_required(VERSION 3.15)
project(Ehu)

# 设置构建类型
set(CMAKE_CONFIGURATION_TYPES Debug Release Dist)

set(BUILD_DIR "${CMAKE_BINARY_DIR}")
set(CMAKE_SKIP_BUILD_RPATH TRUE)

# 添加子项目
add_subdirectory(Ehu)
add_subdirectory(SandBox)

set(CMAKE_BINARY_DIR "build")
set(CMAKE_INSTALL_PREFIX "install")