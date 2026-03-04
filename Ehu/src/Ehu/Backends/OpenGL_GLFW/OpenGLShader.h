#pragma once

#include "Platform/Shader.h"
#include <unordered_map>

namespace Ehu {

	class OpenGLShader : public Shader {
	public:
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		void Bind() const override;
		void Unbind() const override;

		void SetMat4(const std::string& name, const glm::mat4& value) override;
		void SetFloat4(const std::string& name, const glm::vec4& value) override;
		void SetFloat3(const std::string& name, const glm::vec3& value) override;
		void SetFloat(const std::string& name, float value) override;
		void SetInt(const std::string& name, int value) override;

	private:
		int GetUniformLocation(const std::string& name) const;
		uint32_t CompileShader(uint32_t type, const std::string& source);

		uint32_t m_RendererID = 0;
		mutable std::unordered_map<std::string, int> m_UniformLocationCache;
	};

} // namespace Ehu
