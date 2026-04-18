#pragma once

namespace Ehu {

	class EditorSession;

	class HierarchyPanel {
	public:
		bool OnImGuiRender(const EditorSession& session, bool* pOpen);
	};

} // namespace Ehu
