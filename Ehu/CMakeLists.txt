foreach(CONFIG_TYPE Debug Release Dist)
    string(TOUPPER "${CONFIG_TYPE}" CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${BUILD_DIR}/bin/${CONFIG_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${BUILD_DIR}/bin/${CONFIG_TYPE}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${BUILD_DIR}/bin-int/${CONFIG_TYPE}")
endforeach()

# 创建库
add_library(EhuLib SHARED)

# 添加子目录
add_subdirectory(src)


# 设置预编译头
target_precompile_headers(EhuLib PUBLIC "src/Ehu/ehupch.h")

# 包含目录
target_include_directories(EhuLib PRIVATE 
    vendor/spdlog/include
    vendor/glfw/include
)

add_subdirectory(vendor/glfw)  # 添加 GLFW 构建目录
target_link_libraries(EhuLib PRIVATE glfw)

# Windows设置
if(WIN32)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_definitions(-DEHU_PLATFORM_WINDOWS -DEHU_BUILD_DLL)
endif()

# 配置文件类型（Debug, Release, Dist）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DEHU_DEBUG)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
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
