#include "OpenGLRendererAPI.h"
#include "Platform/Render/Resources/VertexArray.h"
#include "Platform/Render/Framebuffer.h"
#include <glad/glad.h>

namespace Ehu {

	void OpenGLRendererAPI::SetClearColor(float r, float g, float b, float a) {
		glClearColor(r, g, b, a);
	}

	void OpenGLRendererAPI::Clear(uint32_t clearFlags) {
		GLbitfield mask = 0;
		if (clearFlags & ClearColor) mask |= GL_COLOR_BUFFER_BIT;
		if (clearFlags & ClearDepth) mask |= GL_DEPTH_BUFFER_BIT;
		if (clearFlags & ClearStencil) mask |= GL_STENCIL_BUFFER_BIT;
		if (mask) glClear(mask);
	}

	void OpenGLRendererAPI::DrawIndexed(VertexArray* vertexArray, uint32_t indexCount) {
		uint32_t count = indexCount > 0 ? indexCount : (vertexArray->GetIndexBuffer() ? vertexArray->GetIndexBuffer()->GetCount() : 0);
		if (count == 0) return;
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::SetDepthTest(bool enable) {
		if (enable) {
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
		} else {
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
		}
	}

	void OpenGLRendererAPI::SetBlend(bool enable) {
		if (enable) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else {
			glDisable(GL_BLEND);
		}
	}

	void OpenGLRendererAPI::SetCullFace(bool enable, bool frontCCW) {
		if (enable) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(frontCCW ? GL_CCW : GL_CW);
		} else {
			glDisable(GL_CULL_FACE);
		}
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
		glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
	}

	void OpenGLRendererAPI::BeginRenderPass(Framebuffer* framebuffer) {
		if (framebuffer)
			framebuffer->Bind();
		else
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLRendererAPI::EndRenderPass() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLRendererAPI::BeginGpuTiming() {
		if (m_GpuTimeQuery == 0)
			glGenQueries(1, &m_GpuTimeQuery);
		if (m_GpuTimeQuery != 0)
			glBeginQuery(GL_TIME_ELAPSED, m_GpuTimeQuery);
	}

	void OpenGLRendererAPI::EndGpuTiming() {
		if (m_GpuTimeQuery != 0)
			glEndQuery(GL_TIME_ELAPSED);
	}

	float OpenGLRendererAPI::GetLastGpuTimeMs() const {
		if (m_GpuTimeQuery == 0) return -1.0f;
		GLuint available = 0;
		glGetQueryObjectuiv(m_GpuTimeQuery, GL_QUERY_RESULT_AVAILABLE, &available);
		if (available) {
			GLuint64 elapsedNs = 0;
			glGetQueryObjectui64v(m_GpuTimeQuery, GL_QUERY_RESULT, &elapsedNs);
			m_LastGpuTimeMs = static_cast<float>(elapsedNs) / 1e6f;
		}
		return m_LastGpuTimeMs;
	}

	uint64_t OpenGLRendererAPI::GetVramBytes() const {
		// NVIDIA: GL_NVX_gpu_memory_info (values in KB)
#ifndef GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX
		#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX 0x9048
#endif
#ifndef GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX
		#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
#endif
		GLint totalKb = 0, availKb = 0;
		glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalKb);
		glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availKb);
		if (totalKb > 0 && availKb >= 0)
			return static_cast<uint64_t>(totalKb - availKb) * 1024u;
		return 0;
	}

} // namespace Ehu
