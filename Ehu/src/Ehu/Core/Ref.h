#pragma once

#include <memory>

namespace Ehu {

	/// shared_ptr 别名，用于共享所有权（多引用计数）
	template<typename T>
	using Ref = std::shared_ptr<T>;

	/// unique_ptr 别名，用于独占所有权（单所有者）
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

} // namespace Ehu
