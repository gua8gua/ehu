#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Texture2D.h"
#include <glm/glm.hpp>

namespace Ehu {

	/// 子纹理：引用一张 Texture2D 的 UV 矩形区域，用于图集内精灵
	class EHU_API SubTexture2D {
	public:
		SubTexture2D(Texture2D* texture, const glm::vec2& uvMin, const glm::vec2& uvMax);

		Texture2D* GetTexture() const { return m_Texture; }
		const glm::vec2& GetUVMin() const { return m_UVMin; }
		const glm::vec2& GetUVMax() const { return m_UVMax; }

		/// 从网格图集按瓦片坐标创建子纹理（tile 从 0 起，cols/rows 为图集列数/行数）
		static SubTexture2D CreateFromCoords(Texture2D* texture,
			uint32_t tileX, uint32_t tileY, uint32_t tileW, uint32_t tileH,
			uint32_t atlasCols, uint32_t atlasRows);

	private:
		Texture2D* m_Texture = nullptr;
		glm::vec2 m_UVMin = { 0.0f, 0.0f };
		glm::vec2 m_UVMax = { 1.0f, 1.0f };
	};

	// --- Inline implementation ---

	inline SubTexture2D::SubTexture2D(Texture2D* texture, const glm::vec2& uvMin, const glm::vec2& uvMax)
		: m_Texture(texture), m_UVMin(uvMin), m_UVMax(uvMax) {}

	inline SubTexture2D SubTexture2D::CreateFromCoords(Texture2D* texture,
		uint32_t tileX, uint32_t tileY, uint32_t tileW, uint32_t tileH,
		uint32_t atlasCols, uint32_t atlasRows) {
		if (!texture || atlasCols == 0 || atlasRows == 0)
			return SubTexture2D(texture, { 0.0f, 0.0f }, { 1.0f, 1.0f });
		float w = 1.0f / static_cast<float>(atlasCols);
		float h = 1.0f / static_cast<float>(atlasRows);
		glm::vec2 uvMin(tileX * w, (atlasRows - 1 - tileY) * h);
		glm::vec2 uvMax((tileX + tileW) * w, (atlasRows - 1 - tileY + tileH) * h);
		return SubTexture2D(texture, uvMin, uvMax);
	}

} // namespace Ehu
