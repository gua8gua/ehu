# 设置构建输出文件
foreach(CONFIG_TYPE Debug Release Dist)
    string(TOUPPER "${CONFIG_TYPE}" CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${BUILD_DIR}/bin/${CONFIG_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${BUILD_DIR}/bin/${CONFIG_TYPE}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${BUILD_DIR}/bin-int/${CONFIG_TYPE}")
endforeach()

# 创建库
add_library(EhuLib STATIC)
add_library(EhuEditorSupportLib STATIC)
add_library(EhuImGuiLib STATIC)

# 添加子目录
add_subdirectory(src)


# 设置预编译头
foreach(EHU_TARGET EhuLib EhuEditorSupportLib EhuImGuiLib)
    target_precompile_headers(${EHU_TARGET} PUBLIC "src/Ehu/ehupch.h")
    set_target_properties(${EHU_TARGET} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )
endforeach()

# 包含目录
# spdlog为头文件库
target_include_directories(EhuLib PRIVATE
    vendor/spdlog/include
    vendor/GLFW/include
    vendor/GLAD/include
    vendor/imgui
    vendor/stb
)
target_compile_definitions(EhuLib PRIVATE EHU_USE_STB_IMAGE)
target_include_directories(EhuEditorSupportLib PRIVATE
    vendor/spdlog/include
    vendor/GLFW/include
    vendor/GLAD/include
)
target_include_directories(EhuImGuiLib PRIVATE
    vendor/spdlog/include
    vendor/GLFW/include
    vendor/GLAD/include
    vendor/imgui
)

# 添加 OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(EhuLib PRIVATE OpenGL::GL)

# 添加 GLFW，内部有cmake直接加
add_subdirectory(vendor/GLFW)
target_link_libraries(EhuLib PRIVATE glfw)

# 添加 GLAD，无cmake手动打包
add_library(glad STATIC vendor/GLAD/src/glad.c)
target_include_directories(glad PUBLIC
        vendor/GLAD/include)
target_link_libraries(EhuLib PRIVATE glad)

# Box2D（与 Hazel 对齐的 2D 物理）
include(FetchContent)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
	box2d
	GIT_REPOSITORY https://github.com/erincatto/box2d.git
	GIT_TAG v2.4.1
)
FetchContent_MakeAvailable(box2d)
target_link_libraries(EhuLib PRIVATE box2d)

# 添加 ImGui
add_subdirectory(vendor/imgui)
target_link_libraries(EhuImGuiLib PUBLIC ImGui EhuLib)  # ImGui 桥接与工具窗口由编辑器显式接入

# 添加glm
add_subdirectory(vendor/glm)
target_link_libraries(EhuLib PUBLIC glm)
target_link_libraries(EhuEditorSupportLib PUBLIC EhuLib)

add_definitions( -DEHU_BUILD_DLL -DEHU_STATIC)

# 预留 Mono 运行时依赖（用于 C# 脚本集成）
# - 优先使用环境变量 MONO_ROOT 指定的安装位置
# - 若未设置，则交由外部工具/用户自行配置 include/lib 路径
set(MONO_ROOT "" CACHE PATH "Root directory of Mono installation (contains include/mono-2.0 and lib)")
if(MONO_ROOT)
    message(STATUS "Mono root: ${MONO_ROOT}")
    set(MONO_INCLUDE_DIR "${MONO_ROOT}/include/mono-2.0")
    set(MONO_LIB_DIR "${MONO_ROOT}/lib")
    find_path(MONO_EMBED_INCLUDE mono/jit/jit.h PATHS "${MONO_INCLUDE_DIR}")
    find_library(MONO_EMBED_LIB NAMES mono-2.0-sgen mono-2.0 PATHS "${MONO_LIB_DIR}")
    if(MONO_EMBED_INCLUDE AND MONO_EMBED_LIB)
        message(STATUS "Found Mono embedding API: ${MONO_EMBED_INCLUDE}, lib: ${MONO_EMBED_LIB}")
        target_include_directories(EhuLib PRIVATE "${MONO_EMBED_INCLUDE}")
        target_link_libraries(EhuLib PRIVATE "${MONO_EMBED_LIB}")
        add_definitions(-DEHU_ENABLE_MONO)
    else()
        message(WARNING "MONO_ROOT is set but mono embedding headers or library were not found. C# 脚本运行时暂不可用。")
    endif()
else()
    message(STATUS "MONO_ROOT not set; 跳过 Mono 链接，脚本运行时需手动配置。")
endif()

# Windows设置
if(WIN32)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_definitions(-DEHU_PLATFORM_WINDOWS)
    target_link_libraries(EhuLib PRIVATE shell32 ole32)  # IFileDialog (FileDialog.cpp)

    # 区分 MSVC 与 MinGW / Clang
    if (MSVC)
        message(STATUS "Compiler: MSVC, enabling /utf-8")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    else()
        message(STATUS "Compiler: MinGW or Clang, enabling UTF-8 charset")
        add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
endif()

# 配置文件类型（Debug, Release, Dist）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DEHU_DEBUG -DEHU_ENABLE_ASSERTS)
    if(NOT MSVC)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DEHU_RELEASE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
elseif(CMAKE_BUILD_TYPE STREQUAL "Dist")
    add_definitions(-DEHU_DIST)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# 自动收集所有子目录下的 .cpp 和 .h 文件
file(GLOB_RECURSE ALL_SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# 宏：按目录自动生成 Visual Studio 分组
macro(source_group_by_dir_recursive files)
    if(MSVC)
        foreach(f ${files})
            # 得到相对于根目录的路径
            file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_SOURCE_DIR}" "${f}")

            # 提取目录部分
            get_filename_component(dir_path "${rel_path}" PATH)
            if(dir_path STREQUAL "")
                set(dir_path "\\")
            else()
                string(REPLACE "/" "\\" dir_path "${dir_path}")
            endif()

            # 创建 source group
            source_group("${dir_path}" FILES "${f}")
        endforeach()
    endif()
endmacro()

# 调用宏生成分组
source_group_by_dir_recursive("${ALL_SRC_FILES}")
