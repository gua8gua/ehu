#pragma once

#include "Core/Core.h"
#include <cstdint>
#include <cstddef>

namespace Ehu {

	/// 仪表盘统一统计：Timing / Rendering / Memory，由引擎各处在每帧写入，DashboardLayer 只读展示
	class EHU_API DashboardStats {
	public:
		static constexpr size_t kHistoryFrames = 120;

		// --- Timing (ms) ---
		float FrameTimeMs = 0.0f;
		float CpuTimeMs = 0.0f;
		float GpuTimeMs = -1.0f;          // 上一帧 GPU 耗时，无查询时 < 0 表示 N/A
		float UpdateMs = 0.0f;
		float PhysicsMs = 0.0f;           // 暂无物理子系统时保持 0
		float RenderSubmitMs = 0.0f;
		float ScriptingMs = 0.0f;         // 暂无脚本子系统时保持 0

		// --- Rendering ---
		uint32_t DrawCalls2D = 0;
		uint32_t DrawCalls3D = 0;
		uint32_t Triangles2D = 0;
		uint32_t Triangles3D = 0;
		uint32_t Vertices2D = 0;          // 近似或从 VAO 累加
		uint32_t Vertices3D = 0;
		uint32_t TextureBindings = 0;     // 上一帧纹理绑定次数（展示用）
		uint32_t ShaderSwitches = 0;      // 上一帧着色器切换次数（展示用）

		// --- Memory & Resources ---
		uint64_t VramBytes = 0;           // 0 表示未实现或 N/A
		uint64_t HeapBytes = 0;           // 进程堆/工作集，0 表示 N/A
		uint32_t ActiveEntities = 0;

		// --- History (ring buffer, for plots) ---
		float FrameTimeHistory[kHistoryFrames] = {};
		float GpuTimeHistory[kHistoryFrames] = {};
		float HeapMbHistory[kHistoryFrames] = {};
		size_t HistoryCount = 0;          // 已写入的帧数，最多 kHistoryFrames

	private:
		uint32_t m_TextureBindingsThisFrame = 0;
		uint32_t m_ShaderSwitchesThisFrame = 0;

	public:
		static DashboardStats& Get() {
			static DashboardStats s_Instance;
			return s_Instance;
		}

		/// 每帧开始时由 Application::Run 调用，将上一帧的纹理/着色器计数写入展示字段并清零
		void SnapshotRenderingCounters() {
			TextureBindings = m_TextureBindingsThisFrame;
			ShaderSwitches = m_ShaderSwitchesThisFrame;
			m_TextureBindingsThisFrame = 0;
			m_ShaderSwitchesThisFrame = 0;
		}

		void AddTextureBinding() { m_TextureBindingsThisFrame++; }
		void AddShaderSwitch() { m_ShaderSwitchesThisFrame++; }

		/// 每帧结束时由采集方调用，将当前帧数据写入历史
		void PushHistory() {
			FrameTimeHistory[HistoryCount % kHistoryFrames] = FrameTimeMs;
			GpuTimeHistory[HistoryCount % kHistoryFrames] = GpuTimeMs >= 0.0f ? GpuTimeMs : 0.0f;
			HeapMbHistory[HistoryCount % kHistoryFrames] = HeapBytes > 0 ? (HeapBytes / (1024.0f * 1024.0f)) : 0.0f;
			if (HistoryCount < kHistoryFrames)
				HistoryCount++;
		}
	};

} // namespace Ehu
