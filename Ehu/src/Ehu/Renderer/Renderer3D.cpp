#include "ehupch.h"
#include "Renderer3D.h"

namespace Ehu {

	void Renderer3D::Init() {
		// TODO: 创建默认 3D Shader、管线等
	}

	void Renderer3D::Shutdown() {
	}

	void Renderer3D::BeginScene(const PerspectiveCamera& camera) {
		(void)camera;
		// TODO: 绑定 Shader、上传 ViewProjection
	}

	void Renderer3D::EndScene() {
		// TODO: 批量提交
	}

	void Renderer3D::Submit(const glm::mat4& transform, const glm::vec4& color) {
		(void)transform;
		(void)color;
		// TODO: 待 3D 管线实现
	}

} // namespace Ehu
