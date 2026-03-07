#pragma once

#include "ehupch.h"
#include "Core/Core.h"

namespace Ehu {

	class EHU_API VertexBuffer {
	public:
		virtual ~VertexBuffer() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void SetData(const void* data, uint32_t size) = 0;

		static VertexBuffer* Create(uint32_t size);
		static VertexBuffer* Create(const void* vertices, uint32_t size);
	};

} // namespace Ehu
