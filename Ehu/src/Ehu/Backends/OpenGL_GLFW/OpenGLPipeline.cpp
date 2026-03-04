#include "OpenGLPipeline.h"
#include "Platform/RendererAPI.h"
#include "Platform/Shader.h"
#include "Platform/VertexArray.h"

namespace Ehu {

	void OpenGLPipeline::SetShader(Shader* shader) { m_Shader = shader; }
	void OpenGLPipeline::SetVertexArray(VertexArray* vertexArray) { m_VertexArray = vertexArray; }
	void OpenGLPipeline::SetDepthTest(bool enable) { m_DepthTest = enable; }
	void OpenGLPipeline::SetBlend(bool enable) { m_Blend = enable; }
	void OpenGLPipeline::SetCullFace(bool enable, bool frontCCW) { m_CullFace = enable; m_FrontCCW = frontCCW; }

	void OpenGLPipeline::Bind() {
		RendererAPI& api = RendererAPI::Get();
		api.SetDepthTest(m_DepthTest);
		api.SetBlend(m_Blend);
		api.SetCullFace(m_CullFace, m_FrontCCW);
		if (m_Shader) m_Shader->Bind();
		if (m_VertexArray) m_VertexArray->Bind();
	}

	void OpenGLPipeline::DrawIndexed(uint32_t indexCount) {
		if (m_VertexArray)
			RendererAPI::Get().DrawIndexed(m_VertexArray, indexCount);
	}

} // namespace Ehu
