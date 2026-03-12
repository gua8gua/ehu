# 编辑器可执行程序（前端模块，依赖 EhuLib）
add_executable(EhuEditor)

add_subdirectory(src)

target_include_directories(EhuEditor PRIVATE
    ${CMAKE_SOURCE_DIR}/Ehu/vendor/spdlog/include
    ${CMAKE_SOURCE_DIR}/Ehu/vendor/imgui
    ${CMAKE_SOURCE_DIR}/Ehu/src
    ${CMAKE_SOURCE_DIR}/Ehu/src/Ehu
)

target_link_libraries(EhuEditor PRIVATE EhuLib)

if(WIN32)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_definitions(-DEHU_PLATFORM_WINDOWS -DEHU_BUILD_DLL)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    else()
        add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DEHU_DEBUG -DEHU_ENABLE_ASSERTS)
endif()
