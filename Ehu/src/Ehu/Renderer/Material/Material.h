#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace Ehu {

	/// 材质属性（平台无关）：颜色、纹理引用等，具体绑定由 Render API 层实现
	struct EHU_API MaterialProperties {
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float Metallic = 0.0f;
		float Roughness = 0.5f;
	};

	/// 材质：持有属性，与 Shader 绑定（Bind 待接 Shader 后补全）
	class EHU_API Material {
	public:
		Material(const std::string& name = "Material");
		virtual ~Material() = default;

		const std::string& GetName() const { return m_Name; }
		MaterialProperties& GetProperties() { return m_Properties; }
		const MaterialProperties& GetProperties() const { return m_Properties; }

		virtual void Bind() const;
		virtual void Unbind() const;

	private:
		std::string m_Name;
		MaterialProperties m_Properties;
	};

} // namespace Ehu
