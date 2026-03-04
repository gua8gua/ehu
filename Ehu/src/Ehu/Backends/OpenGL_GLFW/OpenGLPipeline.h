#pragma once

#include "Platform/Pipeline.h"

namespace Ehu {

	class OpenGLPipeline : public Pipeline {
	public:
		void SetShader(Shader* shader) override;
		void SetVertexArray(VertexArray* vertexArray) override;
		void SetDepthTest(bool enable) override;
		void SetBlend(bool enable) override;
		void SetCullFace(bool enable, bool frontCCW = true) override;

		void Bind() override;
		void DrawIndexed(uint32_t indexCount = 0) override;

	private:
		Shader* m_Shader = nullptr;
		VertexArray* m_VertexArray = nullptr;
		bool m_DepthTest = true;
		bool m_Blend = true;
		bool m_CullFace = false;
		bool m_FrontCCW = true;
	};

} // namespace Ehu
