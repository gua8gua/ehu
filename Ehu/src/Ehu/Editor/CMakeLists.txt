# Editor：引擎侧可被前端调用的部分（无 ImGui）
set(EDITOR_SOURCES
	EditorContext.cpp
	EditorPanelData.cpp
	ViewportRenderer.cpp
)
set(EDITOR_HEADERS
	EditorContext.h
	EditorPanelData.h
	ViewportRenderer.h
)
target_sources(EhuLib PRIVATE ${EDITOR_SOURCES} ${EDITOR_HEADERS})
target_include_directories(EhuLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
