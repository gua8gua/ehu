#include "ehupch.h"
#include "Renderer3D.h"
#include "Platform/Render/RendererAPI.h"
#include "Platform/Render/Resources/Shader.h"
#include "Platform/Render/Resources/VertexArray.h"

namespace Ehu {

	struct SceneData {
		glm::mat4 ViewProjectionMatrix;
	};
	static SceneData* s_SceneData = nullptr;
	static Shader* s_Shader = nullptr;

	void Renderer3D::Init() {
		s_SceneData = new SceneData();
		s_Shader = Shader::CreateDefault3D();
	}

	void Renderer3D::Shutdown() {
		delete s_SceneData;
		s_SceneData = nullptr;
		delete s_Shader;
		s_Shader = nullptr;
	}

	void Renderer3D::BeginScene(const PerspectiveCamera& camera) {
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
		s_Shader->Bind();
		s_Shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
	}

	void Renderer3D::EndScene() {
	}

	void Renderer3D::Submit(VertexArray* vertexArray, uint32_t indexCount, const glm::mat4& transform, const glm::vec4& color) {
		if (!vertexArray || indexCount == 0) return;
		s_Shader->SetMat4("u_Transform", transform);
		s_Shader->SetFloat4("u_Color", color);
		vertexArray->Bind();
		RendererAPI::Get().DrawIndexed(vertexArray, indexCount);
	}

} // namespace Ehu
