#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include <glm/glm.hpp>
#include <string>

namespace Ehu {

	class EHU_API Shader {
	public:
		virtual ~Shader() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetInt(const std::string& name, int value) = 0;

		static Shader* Create(const std::string& vertexSrc, const std::string& fragmentSrc);

		/// 由底层实现持有的默认着色器，Renderer 层仅通过 Bind/SetMat4/SetFloat4 等做设置
		static Shader* CreateDefault2D();
		/// 带纹理采样的 2D 着色器（a_Position, a_Color, a_TexCoord; u_Texture, u_UVMin, u_UVMax, u_Color 作为 tint）
		static Shader* CreateDefault2DTextured();
		static Shader* CreateDefault3D();
	};

} // namespace Ehu
