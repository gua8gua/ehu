#pragma once

#include "Core/Core.h"
#include <cstdint>

namespace Ehu {

	/// ECS 实体句柄：id 可复用，generation 在 Destroy 后自增，用于判无效句柄
	struct EHU_API Entity {
		uint32_t id = 0;
		uint32_t generation = 0;

		bool operator==(const Entity& other) const { return id == other.id && generation == other.generation; }
		bool operator!=(const Entity& other) const { return !(*this == other); }

		explicit operator bool() const { return id != 0 || generation != 0; }
	};

} // namespace Ehu

namespace std {

	template<>
	struct hash<Ehu::Entity> {
		size_t operator()(const Ehu::Entity& e) const {
			return std::hash<uint64_t>()(static_cast<uint64_t>(e.id) | (static_cast<uint64_t>(e.generation) << 32));
		}
	};

} // namespace std
