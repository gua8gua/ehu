#pragma once

#include "Core/Core.h"

namespace Ehu {

	class Project;
	class AssetRegistry;

	/// 项目资源预览面板：树/列表展示资产目录，选中写入 EditorContext
	class ContentBrowserPanel {
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel();

		void OnImGuiRender();

	private:
		AssetRegistry* m_Registry = nullptr;
		bool m_RefreshPending = true;
	};

} // namespace Ehu
