#include "ehupch.h"
#include "OpenGLShader.h"
#include "Core/Log.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Ehu {

	OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc) {
		uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertexSrc);
		uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

		m_RendererID = glCreateProgram();
		glAttachShader(m_RendererID, vs);
		glAttachShader(m_RendererID, fs);
		glLinkProgram(m_RendererID);

		int linked = 0;
		glGetProgramiv(m_RendererID, GL_LINK_STATUS, &linked);
		if (!linked) {
			int length = 0;
			glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &length);
			std::vector<char> log(length);
			glGetProgramInfoLog(m_RendererID, length, &length, log.data());
			EHU_CORE_ERROR("Shader link failed: {0}", log.data());
			glDeleteProgram(m_RendererID);
			m_RendererID = 0;
		}

		glDeleteShader(vs);
		glDeleteShader(fs);
	}

	OpenGLShader::~OpenGLShader() {
		glDeleteProgram(m_RendererID);
	}

	uint32_t OpenGLShader::CompileShader(uint32_t type, const std::string& source) {
		uint32_t id = glCreateShader(type);
		const char* src = source.c_str();
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		int compiled = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			int length = 0;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
			std::vector<char> log(length);
			glGetShaderInfoLog(id, length, &length, log.data());
			EHU_CORE_ERROR("Shader compile failed: {0}", log.data());
			glDeleteShader(id);
			return 0;
		}
		return id;
	}

	void OpenGLShader::Bind() const {
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const {
		glUseProgram(0);
	}

	int OpenGLShader::GetUniformLocation(const std::string& name) const {
		auto it = m_UniformLocationCache.find(name);
		if (it != m_UniformLocationCache.end())
			return it->second;
		int loc = glGetUniformLocation(m_RendererID, name.c_str());
		m_UniformLocationCache[name] = loc;
		return loc;
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value) {
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value) {
		glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value) {
		glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value) {
		glUniform1f(GetUniformLocation(name), value);
	}

	void OpenGLShader::SetInt(const std::string& name, int value) {
		glUniform1i(GetUniformLocation(name), value);
	}

} // namespace Ehu
