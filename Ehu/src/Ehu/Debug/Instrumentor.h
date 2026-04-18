#pragma once

#include "Core/Core.h"

/// 可选 CPU 标记层（占位：接入 Tracy/Chrome 时可将宏映射到真实 API）
#if EHU_DEBUG
#define EHU_PROFILE_FUNCTION()
#define EHU_PROFILE_SCOPE(name)
#else
#define EHU_PROFILE_FUNCTION()
#define EHU_PROFILE_SCOPE(name)
#endif
