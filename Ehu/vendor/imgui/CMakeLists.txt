cmake_minimum_required(VERSION 3.10)

# 设置项目名称和语言
project(ImGui LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 添加源文件
set(IMGUI_SOURCES
        imconfig.h
        imgui.h
        imgui.cpp
        imgui_draw.cpp
        imgui_internal.h
        imgui_tables.cpp
        imgui_widgets.cpp
        imstb_rectpack.h
        imstb_textedit.h
        imstb_truetype.h
        imgui_demo.cpp
)

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/ImGui)
set(CMAKE_OBJECT_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin-int/${CMAKE_BUILD_TYPE}/ImGui)

# 选择编译选项
if(MSVC)
    # Windows 系统
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
    # Linux 系统
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # -fPIC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

# 创建静态库
add_library(ImGui STATIC ${IMGUI_SOURCES})

# Debug 配置
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /D_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Release 配置
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

# Dist 配置（可以根据需要调整）
set(CMAKE_CXX_FLAGS_DIST "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DIST "${CMAKE_CXX_FLAGS_DIST} -g0")

# 如果是 Linux，启用 PIC
if(UNIX)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# 设置系统版本和其他系统特定设置
if(WIN32)
    set(CMAKE_SYSTEM_VERSION "latest")
elseif(UNIX)
    set(CMAKE_SYSTEM_VERSION "latest")
endif()

