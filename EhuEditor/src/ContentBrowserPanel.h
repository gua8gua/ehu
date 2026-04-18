#pragma once

#include "Core/Core.h"

namespace Ehu {

	class Project;
	class AssetRegistry;
	class EditorSession;

	struct ContentBrowserPanelResult {
		/// 双击 .ehuscene 时请求打开（相对 Assets 路径）
		std::string OpenSceneRelativePath;
	};

	/// 项目资源预览面板：树/列表展示资产目录，选中写入 EditorContext
	class ContentBrowserPanel {
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel();

		ContentBrowserPanelResult OnImGuiRender(const EditorSession& session, bool* pOpen);

	private:
		AssetRegistry* m_Registry = nullptr;
		bool m_RefreshPending = true;
		bool m_ShowCreatePopup = false;
		int m_SelectedCreator = 0;
		std::string m_CurrentDirectory;
		char m_NewAssetNameBuf[256] = "NewAsset";
	};

} // namespace Ehu
