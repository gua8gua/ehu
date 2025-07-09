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
)

message(STATUS "EHU_SOURCES = ${EHU_SOURCES}")
add_library(Ehu SHARED ${EHU_SOURCES})

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

    # 后期构建命令
    #   add_custom_command(TARGET Ehu POST_BUILD
    #   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Ehu> ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}-${CMAKE_SYSTEM_NAME}-${CMAKE_ARCHITECTURE}/SandBox
    #   )
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
