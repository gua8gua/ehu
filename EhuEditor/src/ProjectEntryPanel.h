#pragma once

#include <string>

namespace Ehu {

	class EditorSession;

	struct ProjectEntryPanelResult {
		bool RequestNewProject = false;
		bool RequestOpenProject = false;
		std::string RecentProjectToOpen;
	};

	class ProjectEntryPanel {
	public:
		ProjectEntryPanelResult OnImGuiRender(const EditorSession& session) const;
	};

} // namespace Ehu
