#include "InspectorPanel.h"
#include "Editor/EditorPanelData.h"
#include "Project/Project.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Ehu {

	void InspectorPanel::OnImGuiRender() {
		const bool visible = ImGui::Begin("Inspector", nullptr);
		if (!visible) {
			ImGui::End();
			return;
		}

		auto& provider = EditorPanelDataProvider::Get();
		InspectorSnapshot snapshot = provider.GetInspectorSnapshot();

		switch (snapshot.State) {
		case InspectorState::NoSelection:
			if (!Project::GetActive())
				ImGui::TextUnformatted("未加载项目，请先创建或打开项目。");
			else
				ImGui::TextUnformatted("未选择目标，请在 Hierarchy 中选择实体。");
			break;
		case InspectorState::AssetSelection:
			ImGui::TextUnformatted("Asset");
			ImGui::Text("Path: %s", snapshot.AssetPath.c_str());
			break;
		case InspectorState::SelectionInvalid:
			ImGui::TextUnformatted("所选实体已失效，请重新选择。");
			break;
		case InspectorState::EntitySelection: {
			InspectorEntityData& d = snapshot.EntityData;
			if (d.HasTag && ImGui::CollapsingHeader("Tag", ImGuiTreeNodeFlags_DefaultOpen)) {
				char buf[256];
				strncpy_s(buf, d.TagName.c_str(), sizeof(buf) - 1);
				buf[sizeof(buf) - 1] = '\0';
				if (ImGui::InputText("Name", buf, sizeof(buf)))
					provider.SetEntityTag(buf);
			}
			if (d.HasTransform && ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
				glm::vec3 pos = d.Position, scale = d.Scale, rot = d.RotationEulerDeg;
				bool changed = false;
				changed |= ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1f);
				changed |= ImGui::DragFloat3("Rotation", glm::value_ptr(rot), 1.0f);
				changed |= ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.01f, 0.01f);
				if (changed)
					provider.SetEntityTransform(pos, scale, rot);
			}
			if (d.HasSprite && ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
				glm::vec2 size = d.SpriteSize;
				glm::vec4 color = d.SpriteColor;
				bool changed = false;
				changed |= ImGui::DragFloat2("Size", glm::value_ptr(size), 0.01f);
				changed |= ImGui::ColorEdit4("Color", glm::value_ptr(color));
				if (changed)
					provider.SetEntitySprite(size, color);
			}
			if (d.HasMesh && ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Text("IndexCount: %u", d.MeshIndexCount);
				glm::vec4 color = d.MeshColor;
				if (ImGui::ColorEdit4("Color", glm::value_ptr(color)))
					provider.SetEntityMeshColor(color);
			}
			break;
		}
		}

		ImGui::End();
	}

} // namespace Ehu
