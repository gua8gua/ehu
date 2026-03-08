#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Platform/Render/Resources/Texture2D.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace Ehu {

	/// 字体：持有一张图集纹理与每字符的 UV/尺寸/advance，供 Renderer2D::DrawText 使用
	class EHU_API Font {
	public:
		struct GlyphInfo {
			glm::vec2 uvMin = { 0.0f, 0.0f };
			glm::vec2 uvMax = { 1.0f, 1.0f };
			glm::vec2 size = { 0.0f, 0.0f };  // 绘制尺寸（世界/像素）
			float advance = 0.0f;             // 水平步进
		};

		~Font();

		Texture2D* GetAtlasTexture() const { return m_Atlas; }
		float GetPixelHeight() const { return m_PixelHeight; }
		/// 获取字符的 glyph 信息；若不存在则返回 nullptr
		const GlyphInfo* GetGlyph(int codepoint) const;
		/// 获取字符串水平总 advance（简单累加，无 kerning）
		float GetStringAdvance(const std::string& utf8Text) const;

		/// 创建占位字体：单像素白图，ASCII 32–126 等宽；pixelHeight 为行高
		static Font* CreatePlaceholder(float pixelHeight = 32.0f);
		/// 从 TTF 文件创建（若未实现则返回 nullptr）
		static Font* CreateFromFile(const std::string& path, float pixelHeight = 32.0f);

	private:
		Font() = default;
		Texture2D* m_Atlas = nullptr;
		float m_PixelHeight = 32.0f;
		std::unordered_map<int, GlyphInfo> m_Glyphs;
	};

} // namespace Ehu
