foreach(CONFIG_TYPE Debug Release Dist)
    string(TOUPPER "${CONFIG_TYPE}" CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${BUILD_DIR}/bin/${CONFIG_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${BUILD_DIR}/bin/${CONFIG_TYPE}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${BUILD_DIR}/bin-int/${CONFIG_TYPE}")
endforeach()

# 收集所有 .cpp 和 .h 文件
file(GLOB EHU_SOURCES
    "src/*.cpp"
    "src/*.h"
    "src/Ehu/*.cpp"
    "src/Ehu/*.h"
    "src/Ehu/Events/*.cpp"
    "src/Ehu/Events/*.h"
)

# 创建共享库
add_library(Ehu SHARED ${EHU_SOURCES})

# 设置预编译头
target_precompile_headers(Ehu PUBLIC "src/Ehu/ehupch.h")

# 包含目录
target_include_directories(Ehu PRIVATE 
    vendor/spdlog/include
)

# Windows特定设置
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
