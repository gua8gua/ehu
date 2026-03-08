# 核心层：应用、层栈、日志、键鼠码、Ref/Scope、UUID、FileSystem（无平台/渲染依赖）
set(CORE_SOURCES
	Application.cpp
	Layer.cpp
	LayerStack.cpp
	Log.cpp
	UUID.cpp
	FileSystem.cpp
)
set(CORE_HEADERS
	Application.h
	Core.h
	FileSystem.h
	KeyCodes.h
	Layer.h
	LayerStack.h
	Log.h
	MouseCodes.h
	Ref.h
	TimeStep.h
	Timer.h
	UUID.h
)
target_sources(EhuLib PRIVATE ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(EhuLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
