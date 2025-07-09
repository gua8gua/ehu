foreach(CONFIG_TYPE Debug Release Dist)
    string(TOUPPER "${CONFIG_TYPE}" CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${BUILD_DIR}/bin/${CONFIG_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${BUILD_DIR}/bin/${CONFIG_TYPE}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${BUILD_DIR}/bin-int/${CONFIG_TYPE}")
endforeach()

file(GLOB SANDBOX_SOURCES
    "src/*.cpp"
    "src/*.h"
)

add_executable(SandBox ${SANDBOX_SOURCES})

# 包含目录
target_include_directories(SandBox PRIVATE
    ../Ehu/vendor/spdlog/include
    ../Ehu/src
)

# 链接Ehu库
target_link_libraries(SandBox PRIVATE Ehu)

# Windows特定设置
if(WIN32)
    add_definitions(-DEHU_PLATFORM_WINDOWS)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
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
