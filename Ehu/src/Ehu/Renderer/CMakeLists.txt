# 渲染器层（平台无关）：子目录 Camera、Scene、Material；根目录 Renderer2D、Renderer3D、Renderer 入口
set(RENDERER_SOURCES
	Camera/Camera.cpp
	Scene/Scene.cpp
	Material/Material.cpp
	Renderer2D.cpp
	Renderer3D.cpp
	Renderer.cpp
)
set(RENDERER_HEADERS
	Camera/Camera.h
	Scene/Scene.h
	Material/Material.h
	Renderer2D.h
	Renderer3D.h
	Renderer.h
	RendererModule.h
)
target_sources(EhuLib PRIVATE ${RENDERER_SOURCES} ${RENDERER_HEADERS})
target_include_directories(EhuLib PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/Camera
	${CMAKE_CURRENT_SOURCE_DIR}/Scene
	${CMAKE_CURRENT_SOURCE_DIR}/Material
)
