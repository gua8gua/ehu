#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include <string>

namespace Ehu {

	/// 2D 纹理抽象：创建、绑定、设置数据
	class EHU_API Texture2D {
	public:
		virtual ~Texture2D() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;

		/// 从内存创建（data 为 RGBA 像素，nullptr 则仅分配）
		static Texture2D* Create(uint32_t width, uint32_t height, const void* data = nullptr);
		/// 从文件加载（需提供 stb_image 或自行解码后使用 Create）
		static Texture2D* CreateFromFile(const std::string& path);
	};

} // namespace Ehu
