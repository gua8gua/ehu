#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include <memory>
#include <vector>

namespace Ehu {

	class EHU_API VertexArray {
	public:
		virtual ~VertexArray() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(VertexBuffer* vertexBuffer) = 0;
		/// strideFloats: 每顶点 float 数；7 = position(3)+color(4)，9 = position(3)+color(4)+texCoord(2)
		virtual void AddVertexBuffer(VertexBuffer* vertexBuffer, int strideFloats) { (void)strideFloats; AddVertexBuffer(vertexBuffer); }
		virtual void SetIndexBuffer(IndexBuffer* indexBuffer) = 0;

		virtual const std::vector<VertexBuffer*>& GetVertexBuffers() const = 0;
		virtual IndexBuffer* GetIndexBuffer() = 0;

		static VertexArray* Create();
	};

} // namespace Ehu
