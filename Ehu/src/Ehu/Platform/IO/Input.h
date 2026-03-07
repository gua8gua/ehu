#pragma once

#include "ehupch.h"
#include "Core/KeyCodes.h"
#include "Core/MouseCodes.h"

namespace Ehu {

	/// 输入抽象，由 Application 构造时 Init()、析构时 Shutdown()；禁止在 Application 创建前或 Shutdown 后使用
	class Input {
	public:
		static void Init();
		static void Shutdown();

		static bool IsKeyPressed(KeyCode key) { return s_instance ? s_instance->IsKeyPressedImpl(key) : false; }
		static bool IsMouseButtonPressed(MouseCode button) { return s_instance ? s_instance->IsMouseButtonPressedImpl(button) : false; }
		static std::pair<float, float> GetMousePosition() { return s_instance ? s_instance->GetMousePositionImpl() : std::pair<float, float>(0.0f, 0.0f); }
		static float GetMouseX() { return s_instance ? s_instance->GetMouseXImpl() : 0.0f; }
		static float GetMouseY() { return s_instance ? s_instance->GetMouseYImpl() : 0.0f; }
	protected:
		virtual bool IsKeyPressedImpl(const KeyCode& key) = 0;
		virtual bool IsMouseButtonPressedImpl(const MouseCode& button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;
	private:
		static Input* s_instance;
	};

} // namespace Ehu
