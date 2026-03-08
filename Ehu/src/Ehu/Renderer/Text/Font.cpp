#include "ehupch.h"
#include "Font.h"
#include "Platform/Render/Resources/Texture2D.h"

namespace Ehu {

	Font::~Font() {
		if (m_Atlas) {
			delete m_Atlas;
			m_Atlas = nullptr;
		}
	}

	const Font::GlyphInfo* Font::GetGlyph(int codepoint) const {
		auto it = m_Glyphs.find(codepoint);
		if (it != m_Glyphs.end())
			return &it->second;
		return nullptr;
	}

	float Font::GetStringAdvance(const std::string& utf8Text) const {
		float x = 0.0f;
		for (unsigned char c : utf8Text) {
			const GlyphInfo* g = GetGlyph(static_cast<int>(c));
			if (g) x += g->advance;
			else  x += m_PixelHeight * 0.5f;
		}
		return x;
	}

	Font* Font::CreatePlaceholder(float pixelHeight) {
		Font* f = new Font();
		f->m_PixelHeight = pixelHeight;
		// 1x1 白像素作为图集
		uint32_t white = 0xFFFFFFFF;
		f->m_Atlas = Texture2D::Create(1, 1, &white);
		if (!f->m_Atlas) {
			delete f;
			return nullptr;
		}
		float adv = pixelHeight * 0.5f;
		Font::GlyphInfo def;
		def.uvMin = { 0.0f, 0.0f };
		def.uvMax = { 1.0f, 1.0f };
		def.size = { adv, pixelHeight };
		def.advance = adv;
		for (int c = 32; c <= 126; ++c)
			f->m_Glyphs[c] = def;
		// 空格：无绘制尺寸，但有 advance
		f->m_Glyphs[32].size = { 0.0f, 0.0f };
		f->m_Glyphs[32].advance = adv * 0.5f;
		return f;
	}

	Font* Font::CreateFromFile(const std::string& path, float pixelHeight) {
		(void)path;
		(void)pixelHeight;
		return nullptr; // 后续可接 stb_truetype
	}

} // namespace Ehu
