#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Camera/Camera.h"
#include <glm/glm.hpp>

namespace Ehu {
	class Shader;
	class VertexArray;

	/// 2D 渲染器（平台无关）：场景提交接口，具体绘制由 Render API 层实现
	class EHU_API Renderer2D {
	public:
		/// 初始化 2D 渲染器，应在 Application 启动后、首次绘制前调用
		static void Init();
		/// 关闭 2D 渲染器
		static void Shutdown();

		/// 开始一个 2D 场景，使用指定相机
		static void BeginScene(const OrthographicCamera& camera);
		/// 开始场景（任意相机，如 PerspectiveCamera 可做简单 3D 透视）
		static void BeginScene(const Camera& camera);
		/// 结束当前场景，执行批量提交
		static void EndScene();

		/// 提交一个带颜色的四边形（占位，待 VertexBuffer/Shader 实现后接入）
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);

		/// 提交一个带纹理的四边形（占位，待 Texture 抽象实现后接入）
		// static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Texture2D& texture);

	private:
		struct SceneData {
			glm::mat4 ViewProjectionMatrix;
		};
		static SceneData* s_SceneData;
		static Shader* s_Shader;
		static VertexArray* s_QuadVertexArray;
	};

} // namespace Ehu
