# 场景模块：纯数据，不依赖 Renderer
set(SCENE_SOURCES
	Scene.cpp
	SceneSerializer.cpp
)
set(SCENE_HEADERS
	Scene.h
	SceneSerializer.h
)
target_sources(EhuLib PRIVATE ${SCENE_SOURCES} ${SCENE_HEADERS})
target_include_directories(EhuLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
