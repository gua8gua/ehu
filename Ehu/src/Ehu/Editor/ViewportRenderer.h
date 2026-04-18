#pragma once

#include "Core/Core.h"
#include "Core/Ref.h"
#include "ECS/Entity.h"
#include "Renderer/RenderQueue.h"
#include <cstdint>

namespace Ehu {

	class Application;
	class Framebuffer;
	class RenderQueue;
	class EditorCamera;
	class Scene;

	/// 视口渲染：将当前已激活场景按与运行时相同的多场景路径渲染到指定 FBO，使用 EditorCamera 作为视图
	class EHU_API ViewportRenderer {
	public:
		ViewportRenderer();
		~ViewportRenderer();

		/// 设置视口尺寸（创建或调整 FBO，更新 EditorCamera 宽高比）
		void SetSize(uint32_t width, uint32_t height);

		/// 渲染当前帧到视口 FBO（与运行时一致：GetActivatedScenes + SceneLayer SubmitTo + Flush）
		/// overlayScene：编辑器叠加绘制（物理碰撞体线框、选中描边）；可为 nullptr
		void Render(Application& app, Scene* overlayScene = nullptr, bool showPhysicsColliders = false, bool showSelectionOutline = true, Entity selectedForOutline = {});

		/// 获取视口 FBO 的颜色附件纹理 ID（供 ImGui::Image 等使用）
		uint32_t GetColorAttachmentTextureID(uint32_t index = 0) const;
		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		/// 读取第二颜色附件（R32UI）上的实体 id，坐标为左下角原点、与 OpenGL 视口一致
		uint32_t ReadEntityIdAt(uint32_t x, uint32_t y) const;

		const RenderStats& GetLastFrameStats() const { return m_RenderQueue->GetLastFrameStats(); }

		EditorCamera* GetEditorCamera() { return m_EditorCamera.get(); }
		const EditorCamera* GetEditorCamera() const { return m_EditorCamera.get(); }

	private:
		void EnsureFramebuffer(uint32_t width, uint32_t height);

		Framebuffer* m_Framebuffer = nullptr;
		Scope<EditorCamera> m_EditorCamera;
		Scope<RenderQueue> m_RenderQueue;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
	};

} // namespace Ehu
