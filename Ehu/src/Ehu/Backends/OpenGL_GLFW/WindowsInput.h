#pragma once

#include "ehupch.h"
#include "Platform/IO/Input.h"
#include "Core/KeyCodes.h"
#include "Core/MouseCodes.h"

namespace Ehu {

	class WindowsInput : public Input {
	protected:
		bool IsKeyPressedImpl(const KeyCode& key) override;
		bool IsMouseButtonPressedImpl(const MouseCode& button) override;
		std::pair<float, float> GetMousePositionImpl() override;
		float GetMouseXImpl() override;
		float GetMouseYImpl() override;
	};

} // namespace Ehu
