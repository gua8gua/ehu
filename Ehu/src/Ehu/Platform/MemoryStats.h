#pragma once

#include "Core/Core.h"
#include <cstdint>

namespace Ehu {

	/// 平台进程内存统计：当前进程工作集等，不支持平台返回 0
	class EHU_API MemoryStats {
	public:
		/// 当前进程工作集大小（字节），Windows 下为 WorkingSetSize；不支持则返回 0
		static uint64_t GetProcessHeapBytes();
	};

} // namespace Ehu
