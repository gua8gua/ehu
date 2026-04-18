#pragma once

#include "Core.h"
#include <cstddef>
#include <cstdint>

namespace Ehu {

	/// 运行时统计：由引擎核心与后端写入，UI 仅只读展示。
	class EHU_API RuntimeStats {
	public:
		static constexpr size_t kHistoryFrames = 120;

		float FrameTimeMs = 0.0f;
		float CpuTimeMs = 0.0f;
		float GpuTimeMs = -1.0f;
		float UpdateMs = 0.0f;
		float PhysicsMs = 0.0f;
		float RenderSubmitMs = 0.0f;
		float ScriptingMs = 0.0f;

		uint32_t DrawCalls2D = 0;
		uint32_t DrawCalls3D = 0;
		uint32_t Triangles2D = 0;
		uint32_t Triangles3D = 0;
		uint32_t Vertices2D = 0;
		uint32_t Vertices3D = 0;
		uint32_t TextureBindings = 0;
		uint32_t ShaderSwitches = 0;

		uint64_t VramBytes = 0;
		uint64_t HeapBytes = 0;
		uint32_t ActiveEntities = 0;

		float FrameTimeHistory[kHistoryFrames] = {};
		float GpuTimeHistory[kHistoryFrames] = {};
		float HeapMbHistory[kHistoryFrames] = {};
		size_t HistoryCount = 0;

	public:
		static RuntimeStats& Get() {
			static RuntimeStats s_Instance;
			return s_Instance;
		}

		void SnapshotRenderingCounters() {
			TextureBindings = m_TextureBindingsThisFrame;
			ShaderSwitches = m_ShaderSwitchesThisFrame;
			m_TextureBindingsThisFrame = 0;
			m_ShaderSwitchesThisFrame = 0;
		}

		void AddTextureBinding() { m_TextureBindingsThisFrame++; }
		void AddShaderSwitch() { m_ShaderSwitchesThisFrame++; }

		void PushHistory() {
			FrameTimeHistory[HistoryCount % kHistoryFrames] = FrameTimeMs;
			GpuTimeHistory[HistoryCount % kHistoryFrames] = GpuTimeMs >= 0.0f ? GpuTimeMs : 0.0f;
			HeapMbHistory[HistoryCount % kHistoryFrames] = HeapBytes > 0 ? (HeapBytes / (1024.0f * 1024.0f)) : 0.0f;
			if (HistoryCount < kHistoryFrames)
				HistoryCount++;
		}

	private:
		uint32_t m_TextureBindingsThisFrame = 0;
		uint32_t m_ShaderSwitchesThisFrame = 0;
	};

	using DashboardStats = RuntimeStats;

} // namespace Ehu
