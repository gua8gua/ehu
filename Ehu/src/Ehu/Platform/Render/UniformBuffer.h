#pragma once

#include "ehupch.h"
#include "Core/Core.h"

namespace Ehu {

	/// 统一缓冲区抽象：向着色器 uniform 块提供数据（layout(binding=N)）
	class EHU_API UniformBuffer {
	public:
		virtual ~UniformBuffer() = default;

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;

		static UniformBuffer* Create(uint32_t size, uint32_t bindingSlot = 0);
	};

} // namespace Ehu
