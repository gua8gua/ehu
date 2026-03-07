#pragma once

#include "Core.h"

namespace Ehu {

	/// 每帧更新时间增量，供 Layer::OnUpdate 做与帧率无关的更新
	class EHU_API TimeStep {
	public:
		TimeStep() : m_CurrentTime(0.0f), m_LastFrameTime(0.0f) {}

		/// 每帧在 Application::Run() 中调用一次，传入当前时间（秒，通常为自 Run 开始的耗时）
		void Update(float currentTimeSeconds) {
			m_LastFrameTime = m_CurrentTime;
			m_CurrentTime = currentTimeSeconds;
		}

		/// 当前帧开始时刻（秒）
		float GetCurrentTime() const { return m_CurrentTime; }
		/// 上一帧开始时刻（秒）
		float GetLastFrameTime() const { return m_LastFrameTime; }
		/// 本帧与上一帧的时间差（秒），首帧为 0
		float GetDeltaTime() const { return m_CurrentTime - m_LastFrameTime; }

		/// 便捷：当前时间（秒），等价于 GetCurrentTime()
		float GetSeconds() const { return m_CurrentTime; }
		/// 便捷：当前时间（毫秒）
		float GetMilliseconds() const { return m_CurrentTime * 1000.0f; }
		/// 便捷：本帧增量（毫秒）
		float GetDeltaTimeMilliseconds() const { return GetDeltaTime() * 1000.0f; }

	private:
		float m_CurrentTime;
		float m_LastFrameTime;
	};

} // namespace Ehu
