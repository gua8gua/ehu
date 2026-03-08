# ECS 模块：Entity、TypeId、World、Components
set(ECS_SOURCES
	World.cpp
	Components.cpp
)
set(ECS_HEADERS
	Entity.h
	TypeId.h
	World.h
	Components.h
)
target_sources(EhuLib PRIVATE ${ECS_SOURCES} ${ECS_HEADERS})
target_include_directories(EhuLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
