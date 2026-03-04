#pragma once

#include "ehupch.h"
#include "Core/Core.h"

namespace Ehu {

	class Shader;
	class VertexArray;

	/// 渲染管线：捆绑 Shader、VertexArray 与可选渲染状态，Bind 后一次 DrawIndexed 完成从几何到像素的提交
	class EHU_API Pipeline {
	public:
		virtual ~Pipeline() = default;

		virtual void SetShader(Shader* shader) = 0;
		virtual void SetVertexArray(VertexArray* vertexArray) = 0;
		virtual void SetDepthTest(bool enable) = 0;
		virtual void SetBlend(bool enable) = 0;
		virtual void SetCullFace(bool enable, bool frontCCW = true) = 0;

		virtual void Bind() = 0;
		virtual void DrawIndexed(uint32_t indexCount = 0) = 0;

		static Pipeline* Create();
	};

} // namespace Ehu
