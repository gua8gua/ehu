#include "ehupch.h"
#include "Material.h"

namespace Ehu {

	Material::Material(const std::string& name) : m_Name(name) {}

	void Material::Bind() const {
		// 待与 Shader 绑定约定后补全：SetFloat4("u_Color", m_Properties.Color) 等
		(void)m_Properties;
	}

	void Material::Unbind() const {}

} // namespace Ehu
