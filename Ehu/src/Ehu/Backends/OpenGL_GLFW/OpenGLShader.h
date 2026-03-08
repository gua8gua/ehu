#pragma once

#include "Platform/Render/Resources/Shader.h"
#include <unordered_map>

namespace Ehu {

	class OpenGLShader : public Shader {
	public:
		/// 自定义着色器，提供顶点着色器和片段着色器源码
		/// @param vertexSrc 顶点着色器源码
		/// @param fragmentSrc 片段着色器源码
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		/// 底层持有的默认 2D/3D 着色器（position+color, u_ViewProjection/u_Transform/u_Color）
		static Shader* CreateDefault2D();
		static Shader* CreateDefault2DTextured();
		static Shader* CreateDefault3D();

		void Bind() const override;
		void Unbind() const override;

		/// 设置uniform
		/// @param name uniform名称
		/// @param value 值
		void SetMat4(const std::string& name, const glm::mat4& value) override;
		void SetFloat4(const std::string& name, const glm::vec4& value) override;
		void SetFloat3(const std::string& name, const glm::vec3& value) override;
		void SetFloat2(const std::string& name, const glm::vec2& value) override;
		void SetFloat(const std::string& name, float value) override;
		void SetInt(const std::string& name, int value) override;

	private:
		int GetUniformLocation(const std::string& name) const;
		uint32_t CompileShader(uint32_t type, const std::string& source);

		uint32_t m_RendererID = 0;
		mutable std::unordered_map<std::string, int> m_UniformLocationCache;
	};

} // namespace Ehu
