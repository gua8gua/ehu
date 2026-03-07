#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Camera/Camera.h"
#include <glm/glm.hpp>

namespace Ehu {

	class VertexArray;

	/// 3D 渲染器（平台无关）：只负责接收要绘制的几何与变换，不持有具体网格数据
	class EHU_API Renderer3D {
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const PerspectiveCamera& camera);
		static void EndScene();

		/// 提交一次绘制：绑定 3D 默认 Shader，使用传入的 VAO 与索引数、变换与颜色绘制
		/// 顶点布局须与 3D Shader 一致：location 0 = vec3 position, location 1 = vec4 color
		static void Submit(VertexArray* vertexArray, uint32_t indexCount, const glm::mat4& transform, const glm::vec4& color);
	};

} // namespace Ehu
