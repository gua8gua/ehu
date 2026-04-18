#include "ehupch.h"
#include "OpenGLTexture2D.h"
#include "Core/Log.h"
#include "Core/RuntimeStats.h"
#include <glad/glad.h>
#ifdef EHU_USE_STB_IMAGE
	#define STB_IMAGE_IMPLEMENTATION
	#include "stb_image.h"
#endif

namespace Ehu {

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, const void* data) : m_Width(width), m_Height(height) {
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path) : m_Path(path) {
#ifdef EHU_USE_STB_IMAGE
		stbi_set_flip_vertically_on_load(1);
		int w, h, channels;
		unsigned char* img = stbi_load(path.c_str(), &w, &h, &channels, 4);
		if (!img) {
			EHU_CORE_ERROR("Failed to load texture: {0}", path);
			return;
		}
		m_Width = static_cast<uint32_t>(w);
		m_Height = static_cast<uint32_t>(h);
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(img);
#else
		EHU_CORE_WARN("Texture2D from file requires EHU_USE_STB_IMAGE and vendor/stb/stb_image.h");
		(void)path;
#endif
	}

	OpenGLTexture2D::~OpenGLTexture2D() {
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size) {
		uint32_t bpp = 4; // RGBA
		if (size != m_Width * m_Height * bpp) return;
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const {
		if (m_RendererID == 0) return;
		RuntimeStats::Get().AddTextureBinding();
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

} // namespace Ehu
