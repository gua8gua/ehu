#pragma once

#include "Core/Core.h"
#include <cstdint>
#include <typeindex>
#include <typeinfo>

namespace Ehu {

	using ComponentTypeId = uint32_t;

	namespace detail {
		inline ComponentTypeId& NextTypeId() {
			static ComponentTypeId s_Next = 0;
			return s_Next;
		}
	}

	/// 获取组件类型的唯一 ID（用于 ECS 存储与查询）
	template<typename T>
	inline ComponentTypeId GetTypeId() {
		static const ComponentTypeId id = detail::NextTypeId()++;
		return id;
	}

	/// 获取组件类型名（调试/序列化用），默认返回 typeid 名
	template<typename T>
	inline const char* GetTypeName() {
		return typeid(T).name();
	}

} // namespace Ehu
