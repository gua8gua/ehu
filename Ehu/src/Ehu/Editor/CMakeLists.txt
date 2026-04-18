# EditorSupport：引擎侧可被前端调用的编辑器支持层（无 ImGui 窗口编排）
set(EDITOR_SOURCES
	EditorContext.cpp
	EditorPanelData.cpp
	CreationRegistry.cpp
	ViewportRenderer.cpp
)
set(EDITOR_HEADERS
	EditorContext.h
	EditorPanelData.h
	CreationRegistry.h
	ViewportRenderer.h
)
target_sources(EhuEditorSupportLib PRIVATE ${EDITOR_SOURCES} ${EDITOR_HEADERS})
target_include_directories(EhuEditorSupportLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
