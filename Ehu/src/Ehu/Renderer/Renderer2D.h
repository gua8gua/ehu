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
		struct Statistics {
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;
			uint32_t LineCount = 0;
			uint32_t CircleCount = 0;
			uint32_t TextGlyphCount = 0;
		};

		/// 初始化 2D 渲染器，应在 Application 启动后、首次绘制前调用
		static void Init();
		/// 关闭 2D 渲染器
		static void Shutdown();

		/// 开始一个 2D 场景，使用指定相机
		static void BeginScene(const OrthographicCamera& camera);
		/// 开始场景（任意相机，如 PerspectiveCamera 可做简单 3D 透视）
		static void BeginScene(const Camera& camera);
		/// 结束当前场景，执行批量提交（Flush 未满批）
		static void EndScene();

		/// 为第二颜色附件（R32UI）写入实体 id；与 Framebuffer MRT 及视口拾取配合使用
		static void SetEntityIdPassEnabled(bool enabled);
		static bool IsEntityIdPassEnabled();

		/// 本场景（自上次 BeginScene 以来）因批渲染产生的实际 Draw Call 数，供 RenderQueue 统计
		static uint32_t GetBatchDrawCallsThisScene();
		static const Statistics& GetStats();
		static void ResetStats();

		/// 提交带颜色的四边形（纯色路径走批渲染；满批或 EndScene 时 Flush）
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, uint32_t entityId = 0);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, uint32_t entityId = 0);
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, uint32_t entityId = 0);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotationRadians, const glm::vec4& color, uint32_t entityId = 0);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotationRadians, const glm::vec4& color, uint32_t entityId = 0);

		/// 提交带纹理的四边形（tint 默认白）
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, Texture2D* texture, const glm::vec4& tint = glm::vec4(1.0f), uint32_t entityId = 0);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, Texture2D* texture, const glm::vec4& tint = glm::vec4(1.0f), uint32_t entityId = 0);
		static void DrawQuad(const glm::mat4& transform, Texture2D* texture, const glm::vec4& tint = glm::vec4(1.0f), float tilingFactor = 1.0f, uint32_t entityId = 0);
		/// 提交带子纹理的四边形
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const SubTexture2D& subTexture, const glm::vec4& tint = glm::vec4(1.0f), uint32_t entityId = 0);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const SubTexture2D& subTexture, const glm::vec4& tint = glm::vec4(1.0f), uint32_t entityId = 0);
		static void DrawQuad(const glm::mat4& transform, const SubTexture2D& subTexture, const glm::vec4& tint = glm::vec4(1.0f), uint32_t entityId = 0);

		static void DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color, float thickness = 0.05f, uint32_t entityId = 0);
		static void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color, float thickness = 0.05f, uint32_t entityId = 0);
		static void DrawRect(const glm::vec2& center, const glm::vec2& size, const glm::vec4& color, float thickness = 0.05f);
		static void DrawRect(const glm::vec3& center, const glm::vec2& size, const glm::vec4& color, float thickness = 0.05f);
		static void DrawCircle(const glm::vec2& center, float radius, const glm::vec4& color, float thickness = 0.05f, uint32_t segments = 32, uint32_t entityId = 0);
		static void DrawCircle(const glm::vec3& center, float radius, const glm::vec4& color, float thickness = 0.05f, uint32_t segments = 32, uint32_t entityId = 0);

		/// 文本绘制（按字符逐四边形，使用字体的图集与 glyph 信息）
		static void DrawText(const glm::vec2& position, const std::string& text, Font* font, const glm::vec4& tint = glm::vec4(1.0f), uint32_t entityId = 0);
		static void DrawText(const glm::vec3& position, const std::string& text, Font* font, const glm::vec4& tint = glm::vec4(1.0f), uint32_t entityId = 0);

		/// 批渲染：纯色四边形合并为单次 Draw，每批最大四边形数
		static constexpr uint32_t kMaxBatchQuads = 10000;

	private:
		/// 将当前批顶点上传并提交一次 DrawIndexed，然后清空批计数
		static void FlushBatch();

		struct SceneData {
			glm::mat4 ViewProjectionMatrix;
		};
		static SceneData* s_SceneData;
		static Shader* s_Shader;
		static Shader* s_TexturedShader;
		static VertexArray* s_QuadVertexArray;
		static VertexArray* s_QuadTexturedVertexArray;
		static Statistics* s_Stats;
		static bool s_EntityIdPassEnabled;
	};

} // namespace Ehu
