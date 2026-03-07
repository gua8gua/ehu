#pragma once

#include "Core.h"
#include <chrono>

namespace Ehu {

	/// 简易计时器：自 Reset() 或构造以来的经过时间，用于性能统计或延迟逻辑；计时由调用方自行管理
	class EHU_API Timer {
	public:
		Timer() { Reset(); }

		/// 将起始时刻设为当前时刻
		void Reset() { m_Start = std::chrono::steady_clock::now(); }

		/// 自上次 Reset() 以来经过的秒数
		float ElapsedSec() const {
			return std::chrono::duration<float>(std::chrono::steady_clock::now() - m_Start).count();
		}
		/// 自上次 Reset() 以来经过的毫秒数
		float ElapsedMs() const { return ElapsedSec() * 1000.0f; }

	private:
		std::chrono::steady_clock::time_point m_Start;
	};

} // namespace Ehu
