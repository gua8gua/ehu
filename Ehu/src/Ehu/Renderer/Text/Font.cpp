#include "ehupch.h"
#include "Font.h"
#include "Core/FileSystem.h"
#include "Core/Log.h"
#include "Platform/Render/Resources/Texture2D.h"
#include <fstream>
#include <vector>

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include <imstb_truetype.h>

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
		std::ifstream input(path, std::ios::binary | std::ios::ate);
		if (!input) {
			EHU_CORE_WARN("[Font] Failed to open '{}', falling back to placeholder font", path);
			return CreatePlaceholder(pixelHeight);
		}

		const std::streamsize size = input.tellg();
		if (size <= 0) {
			EHU_CORE_WARN("[Font] Empty font file '{}', falling back to placeholder font", path);
			return CreatePlaceholder(pixelHeight);
		}

		input.seekg(0, std::ios::beg);
		std::vector<unsigned char> ttfData(static_cast<size_t>(size));
		if (!input.read(reinterpret_cast<char*>(ttfData.data()), size)) {
			EHU_CORE_WARN("[Font] Failed to read '{}', falling back to placeholder font", path);
			return CreatePlaceholder(pixelHeight);
		}

		const int atlasWidth = 512;
		const int atlasHeight = 512;
		std::vector<unsigned char> bitmap(static_cast<size_t>(atlasWidth * atlasHeight), 0);
		stbtt_bakedchar bakedChars[95] = {};
		const int bakeResult = stbtt_BakeFontBitmap(ttfData.data(), 0, pixelHeight, bitmap.data(), atlasWidth, atlasHeight, 32, 95, bakedChars);
		if (bakeResult <= 0) {
			EHU_CORE_WARN("[Font] Failed to bake '{}', falling back to placeholder font", path);
			return CreatePlaceholder(pixelHeight);
		}

		std::vector<uint8_t> rgba(static_cast<size_t>(atlasWidth * atlasHeight * 4), 255);
		for (int y = 0; y < atlasHeight; ++y) {
			for (int x = 0; x < atlasWidth; ++x) {
				const size_t pixelIndex = static_cast<size_t>(y * atlasWidth + x);
				const uint8_t alpha = bitmap[pixelIndex];
				rgba[pixelIndex * 4 + 0] = 255;
				rgba[pixelIndex * 4 + 1] = 255;
				rgba[pixelIndex * 4 + 2] = 255;
				rgba[pixelIndex * 4 + 3] = alpha;
			}
		}

		Font* font = new Font();
		font->m_PixelHeight = pixelHeight;
		font->m_Atlas = Texture2D::Create(atlasWidth, atlasHeight, rgba.data());
		if (!font->m_Atlas) {
			delete font;
			EHU_CORE_WARN("[Font] Failed to create atlas texture for '{}', falling back to placeholder font", path);
			return CreatePlaceholder(pixelHeight);
		}

		for (int index = 0; index < 95; ++index) {
			const int codepoint = 32 + index;
			const stbtt_bakedchar& glyph = bakedChars[index];
			GlyphInfo info;
			info.uvMin = { glyph.x0 / static_cast<float>(atlasWidth), glyph.y0 / static_cast<float>(atlasHeight) };
			info.uvMax = { glyph.x1 / static_cast<float>(atlasWidth), glyph.y1 / static_cast<float>(atlasHeight) };
			info.size = { static_cast<float>(glyph.x1 - glyph.x0), static_cast<float>(glyph.y1 - glyph.y0) };
			info.bearing = { glyph.xoff, glyph.yoff };
			info.advance = glyph.xadvance;
			font->m_Glyphs[codepoint] = info;
		}

		return font;
	}

} // namespace Ehu
