#pragma once

#include "Core/Core.h"

namespace Ehu {

	/// C++ 与 C# 之间的内部调用绑定（Mono 内部调用表）
	class EHU_API ScriptGlue {
	public:
		static void RegisterInternalCalls();
	};

} // namespace Ehu

