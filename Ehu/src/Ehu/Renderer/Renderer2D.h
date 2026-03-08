#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Camera/Camera.h"
#include <glm/glm.hpp>

namespace Ehu {
	class Shader;
	class VertexArray;
	class Texture2D;
	class SubTexture2D;
	class Font;

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

		/// 提交带颜色的四边形
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);

		/// 提交带纹理的四边形（tint 默认白）
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, Texture2D* texture, const glm::vec4& tint = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, Texture2D* texture, const glm::vec4& tint = glm::vec4(1.0f));
		/// 提交带子纹理的四边形
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const SubTexture2D& subTexture, const glm::vec4& tint = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const SubTexture2D& subTexture, const glm::vec4& tint = glm::vec4(1.0f));

		/// 文本绘制（按字符逐四边形，使用字体的图集与 glyph 信息）
		static void DrawText(const glm::vec2& position, const std::string& text, Font* font, const glm::vec4& tint = glm::vec4(1.0f));
		static void DrawText(const glm::vec3& position, const std::string& text, Font* font, const glm::vec4& tint = glm::vec4(1.0f));

	private:
		struct SceneData {
			glm::mat4 ViewProjectionMatrix;
		};
		static SceneData* s_SceneData;
		static Shader* s_Shader;
		static Shader* s_TexturedShader;
		static VertexArray* s_QuadVertexArray;
		static VertexArray* s_QuadTexturedVertexArray;
	};

} // namespace Ehu
