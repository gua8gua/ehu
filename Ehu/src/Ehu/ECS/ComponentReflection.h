#pragma once

#include "Core/Core.h"
#include "Entity.h"
#include "TypeId.h"
#include <vector>

namespace Ehu {

	class World;

	struct EHU_API ComponentCloneDescriptor {
		const char* Name = "";
		ComponentTypeId Type = 0;
		void (*CloneFn)(const World& srcWorld, Entity srcEntity, World& dstWorld, Entity dstEntity) = nullptr;
	};

	class EHU_API ComponentReflectionRegistry {
	public:
		static const std::vector<ComponentCloneDescriptor>& GetCloneDescriptors();
	};

} // namespace Ehu
