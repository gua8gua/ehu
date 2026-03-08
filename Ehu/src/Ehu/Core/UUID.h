#pragma once

#include "Core.h"
#include <cstdint>
#include <string>

namespace Ehu {

	/// 64 位全局唯一 ID，用于实体、资源引用与序列化
	class EHU_API UUID {
	public:
		UUID();
		UUID(uint64_t raw);
		UUID(const UUID&) = default;

		uint64_t Raw() const { return m_UUID; }
		operator uint64_t() const { return m_UUID; }
		bool operator==(const UUID& other) const { return m_UUID == other.m_UUID; }
		bool operator!=(const UUID& other) const { return m_UUID != other.m_UUID; }

		std::string ToString() const;

	private:
		uint64_t m_UUID;
	};

} // namespace Ehu

namespace std {
	template<>
	struct hash<Ehu::UUID> {
		size_t operator()(const Ehu::UUID& uuid) const {
			return hash<uint64_t>()(uuid.Raw());
		}
	};
}
