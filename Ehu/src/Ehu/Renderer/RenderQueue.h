#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include <glm/glm.hpp>
#include <vector>

namespace Ehu {

	class VertexArray;
	class Camera;

	/// 单条 2D 四边形绘制命令（由实体生成或用户直接提交）
	struct DrawCommand2D {
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec2 Size = { 1.0f, 1.0f };
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float SortKey = 0.0f;   /// 深度/排序键，不透明 Front-to-Back 升序、透明 Back-to-Front 降序
		bool Transparent = false;
		uint32_t LayerIndex = 0;   /// 提交者所在层栈下标
		uint32_t MaterialKey = 0;  /// Phase3 排序用：Shader/Material 分组，最小化状态切换
		Camera* ViewCamera = nullptr; /// 本命令所属视图（由 Scene 的主相机提供）；Dispatch 阶段按相机分批 Begin/Flush/End
	};

	/// 单条 3D 网格绘制命令（由实体生成或用户直接提交）
	struct DrawCommand3D {
		VertexArray* VAO = nullptr;
		uint32_t IndexCount = 0;
		glm::mat4 Transform = glm::mat4(1.0f);
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float SortKey = 0.0f;   /// 深度键，不透明升序（Front-to-Back）、透明降序（Back-to-Front）
		bool Transparent = false;
		uint32_t LayerIndex = 0;
		uint32_t MaterialKey = 0;  /// Phase3：Shader/Material 分组
		Camera* ViewCamera = nullptr;
	};

	/// 渲染队列：收集 2D/3D 绘制命令，排序后统一提交；支持不透明/透明分离与混合
	class EHU_API RenderQueue {
	public:
		void Reserve2D(size_t count);
		void Reserve3D(size_t count);

		/// 设置当前提交所属的层索引（Application 在每层 SubmitTo 前设置），用于同场景多层的绘制顺序
		void SetCurrentLayerIndex(uint32_t index) { m_CurrentLayerIndex = index; }

		/// 提交 2D 四边形（sortKey 用于排序，transparent 为 true 时启用混合并参与透明排序）
		void SubmitQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color,
			float sortKey = 0.0f, bool transparent = false, Camera* viewCamera = nullptr, uint32_t materialKey = 0);
		void SubmitQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color,
			float sortKey = 0.0f, bool transparent = false, Camera* viewCamera = nullptr, uint32_t materialKey = 0);

		/// 提交 3D 网格（vao 与 indexCount 须与 Renderer3D 默认 Shader 布局一致）
		void SubmitMesh(VertexArray* vertexArray, uint32_t indexCount, const glm::mat4& transform, const glm::vec4& color,
			float sortKey = 0.0f, bool transparent = false, Camera* viewCamera = nullptr, uint32_t materialKey = 0);

		/// 排序：先不透明（sortKey 升序），再透明（sortKey 降序，远到近）
		void Sort();

		/// 清空队列（不释放内存，便于复用）
		void Clear();

		bool Has2DCommands() const { return !m_Commands2D.empty(); }
		bool Has3DCommands() const { return !m_Commands3D.empty(); }

		/// 将 2D 命令按排序结果提交到 Renderer2D（调用方需已 BeginScene(camera)，Flush 仅提交绘制与混合状态）
		void Flush2D() const;

		/// 将 3D 命令按排序结果提交到 Renderer3D（调用方需已 BeginScene(camera)）
		void Flush3D() const;

		/// 按 ViewCamera 分批 Begin/Flush/End（场景主相机）；没有主相机的命令不会被绘制
		void FlushAll() const;

	private:
		uint32_t m_CurrentLayerIndex = 0;
		std::vector<DrawCommand2D> m_Commands2D;
		std::vector<DrawCommand3D> m_Commands3D;
		std::vector<size_t> m_SortedOpaque2D;
		std::vector<size_t> m_SortedTransparent2D;
		std::vector<size_t> m_SortedOpaque3D;
		std::vector<size_t> m_SortedTransparent3D;
	};

} // namespace Ehu
