# 渲染器层（平台无关）：子目录 Camera、Material；依赖 Scene 模块
set(RENDERER_SOURCES
	Camera/Camera.cpp
	Camera/EditorCamera.cpp
	Camera/OrthographicCameraController.cpp
	Material/Material.cpp
	RenderQueue.cpp
	Renderer2D.cpp
	Renderer3D.cpp
	Renderer.cpp
	SceneLayer.cpp
	Text/Font.cpp
)
set(RENDERER_HEADERS
	Camera/Camera.h
	Camera/EditorCamera.h
	Camera/OrthographicCameraController.h
	Drawable.h
	Material/Material.h
	RenderQueue.h
	Renderer2D.h
	Renderer3D.h
	Renderer.h
	SceneLayer.h
	RendererModule.h
	Text/Font.h
)
target_sources(EhuLib PRIVATE ${RENDERER_SOURCES} ${RENDERER_HEADERS})
target_include_directories(EhuLib PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/Camera
	${CMAKE_CURRENT_SOURCE_DIR}/Material
	${CMAKE_CURRENT_SOURCE_DIR}/Text
	${CMAKE_CURRENT_SOURCE_DIR}/..   # 以便 #include "Scene/Scene.h"
)
