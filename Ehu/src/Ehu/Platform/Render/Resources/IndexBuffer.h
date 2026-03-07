#pragma once

#include "ehupch.h"
#include "Core/Core.h"

namespace Ehu {

	class EHU_API IndexBuffer {
	public:
		virtual ~IndexBuffer() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual uint32_t GetCount() const = 0;

		static IndexBuffer* Create(const uint32_t* indices, uint32_t count);
	};

} // namespace Ehu
