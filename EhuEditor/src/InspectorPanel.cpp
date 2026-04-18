#include "InspectorPanel.h"
#include "EditorSession.h"
#include "Editor/EditorPanelData.h"
#include "ECS/LayerRegistry.h"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

namespace Ehu {

	void InspectorPanel::OnImGuiRender(const EditorSession& session, bool* pOpen) {
		if (pOpen && !*pOpen)
			return;
		const bool visible = ImGui::Begin("Inspector", pOpen);
		if (!visible) {
			ImGui::End();
			return;
		}

		auto& provider = EditorPanelDataProvider::Get();
		InspectorSnapshot snapshot = provider.GetInspectorSnapshot();

		switch (snapshot.State) {
		case InspectorState::NoSelection:
			if (!session.HasProject())
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
			if (!snapshot.AllowComponentEdit)
				ImGui::TextDisabled("运行时仅查看组件，停止后可编辑。");
			if (!snapshot.AllowComponentEdit)
				ImGui::BeginDisabled();
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
				char texBuf[512]{};
				strncpy_s(texBuf, d.SpriteTexturePath.c_str(), sizeof(texBuf) - 1);
				float tiling = d.SpriteTilingFactor;
				bool changed = false;
				changed |= ImGui::DragFloat2("Size", glm::value_ptr(size), 0.01f);
				changed |= ImGui::ColorEdit4("Color", glm::value_ptr(color));
				if (ImGui::InputText("Texture (Assets 相对路径)", texBuf, sizeof(texBuf)))
					changed = true;
				if (ImGui::DragFloat("Tiling", &tiling, 0.05f, 0.01f, 256.0f))
					changed = true;
				if (changed) {
					provider.SetEntitySprite(size, color);
					provider.SetEntitySpriteTexture(texBuf, tiling);
				}
			}
			if (d.HasCircleRenderer && ImGui::CollapsingHeader("Circle Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
				glm::vec4 c = d.CircleColor;
				float th = d.CircleThickness;
				float fd = d.CircleFade;
				bool ch = false;
				ch |= ImGui::ColorEdit4("Color", glm::value_ptr(c));
				ch |= ImGui::DragFloat("Thickness", &th, 0.001f, 0.001f, 1.0f);
				ch |= ImGui::DragFloat("Fade", &fd, 0.0001f, 0.0f, 0.1f);
				if (ch)
					provider.SetEntityCircleRenderer(c, th, fd);
			}
			if (d.HasText && ImGui::CollapsingHeader("Text", ImGuiTreeNodeFlags_DefaultOpen)) {
				char textBuf[1024]{};
				char fontBuf[512]{};
				strncpy_s(textBuf, d.TextString.c_str(), sizeof(textBuf) - 1);
				strncpy_s(fontBuf, d.TextFontPath.c_str(), sizeof(fontBuf) - 1);
				float ph = d.TextPixelHeight;
				float kern = d.TextKerning;
				float lineSp = d.TextLineSpacing;
				glm::vec4 tc = d.TextColor;
				bool ch = false;
				ch |= ImGui::InputTextMultiline("Text", textBuf, sizeof(textBuf));
				ch |= ImGui::InputText("Font", fontBuf, sizeof(fontBuf));
				ch |= ImGui::DragFloat("PixelHeight", &ph, 0.5f, 4.0f, 256.0f);
				ch |= ImGui::DragFloat("Kerning", &kern, 0.1f);
				ch |= ImGui::DragFloat("LineSpacing", &lineSp, 0.1f);
				ch |= ImGui::ColorEdit4("Color", glm::value_ptr(tc));
				if (ch)
					provider.SetEntityText(textBuf, fontBuf, ph, kern, lineSp, tc);
			}
			if (d.HasCamera && ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
				bool prim = d.CameraPrimary;
				bool fixedA = d.CameraFixedAspect;
				const bool c0 = ImGui::Checkbox("Primary", &prim);
				const bool c1 = ImGui::Checkbox("FixedAspectRatio", &fixedA);
				if (c0 || c1)
					provider.SetEntityCameraFlags(prim, fixedA);
			}
			if (d.HasMesh && ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Text("IndexCount: %u", d.MeshIndexCount);
				glm::vec4 color = d.MeshColor;
				if (ImGui::ColorEdit4("Color", glm::value_ptr(color)))
					provider.SetEntityMeshColor(color);
			}
			if (ImGui::CollapsingHeader("Render Filter", ImGuiTreeNodeFlags_DefaultOpen)) {
				auto channels = LayerRegistry::GetRenderChannels();
				std::vector<std::string> labels;
				labels.reserve(channels.size());
				int currentIndex = 0;
				for (size_t i = 0; i < channels.size(); ++i) {
					labels.push_back(channels[i].second);
					if (channels[i].first == d.RenderChannelValue)
						currentIndex = static_cast<int>(i);
				}
				std::vector<const char*> cLabels;
				cLabels.reserve(labels.size());
				for (const std::string& label : labels)
					cLabels.push_back(label.c_str());
				if (!d.HasRenderFilter)
					ImGui::TextUnformatted("未配置 RenderFilter，修改后将自动创建。");
				if (!cLabels.empty() && ImGui::Combo("RenderChannel", &currentIndex, cLabels.data(), static_cast<int>(cLabels.size())))
					provider.SetEntityRenderChannel(channels[static_cast<size_t>(currentIndex)].first);
			}
			if (ImGui::CollapsingHeader("Physics Filter", ImGuiTreeNodeFlags_DefaultOpen)) {
				uint32_t layerValue = d.CollisionLayer;
				uint32_t maskValue = d.CollisionMask;
				auto physicsLayers = LayerRegistry::GetCollisionLayers();
				int currentLayerIndex = 0;
				for (size_t i = 0; i < physicsLayers.size(); ++i) {
					if (physicsLayers[i].first == layerValue) {
						currentLayerIndex = static_cast<int>(i);
						break;
					}
				}
				if (!d.HasPhysicsFilter)
					ImGui::TextUnformatted("未配置 PhysicsFilter，修改后将自动创建。");

				std::vector<std::string> layerLabels;
				layerLabels.reserve(physicsLayers.size());
				std::vector<const char*> cLayerLabels;
				cLayerLabels.reserve(physicsLayers.size());
				for (const auto& layerInfo : physicsLayers) {
					const uint32_t bit = layerInfo.first;
					const std::string& name = layerInfo.second;
					layerLabels.push_back(name + " (" + std::to_string(bit) + ")");
				}
				for (const std::string& label : layerLabels)
					cLayerLabels.push_back(label.c_str());

				bool changed = false;
				if (!cLayerLabels.empty() && ImGui::Combo("CollisionLayer", &currentLayerIndex, cLayerLabels.data(), static_cast<int>(cLayerLabels.size()))) {
					layerValue = physicsLayers[static_cast<size_t>(currentLayerIndex)].first;
					changed = true;
				}
				for (const auto& layerInfo : physicsLayers) {
					const uint32_t bit = layerInfo.first;
					const std::string& name = layerInfo.second;
					bool enabled = (maskValue & bit) != 0u;
					if (ImGui::Checkbox(name.c_str(), &enabled)) {
						if (enabled) maskValue |= bit;
						else maskValue &= ~bit;
						changed = true;
					}
				}
				ImGui::Text("RenderChannelId: %u", d.RenderChannelValue);
				if (changed) {
					provider.SetEntityPhysicsFilter(layerValue, maskValue);
				}
			}
			if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen)) {
				static char s_NewScriptClassBuf[256] = "Game.";
				if (ImGui::InputText("New Script Class", s_NewScriptClassBuf, sizeof(s_NewScriptClassBuf))) {}
				ImGui::SameLine();
				if (ImGui::Button("Add Script")) {
					provider.AddEntityScriptInstance(s_NewScriptClassBuf);
				}

				if (!d.HasScript || d.ScriptInstances.empty()) {
					ImGui::TextUnformatted("未挂载脚本实例。");
				} else {
					for (size_t i = 0; i < d.ScriptInstances.size(); ++i) {
						const InspectorScriptInstanceData& instance = d.ScriptInstances[i];
						std::string header = instance.ClassName + "##" + instance.InstanceId.ToString();
						if (ImGui::TreeNode(header.c_str())) {
							ImGui::Text("InstanceId: %s", instance.InstanceId.ToString().c_str());
							if (ImGui::Button(("Up##" + instance.InstanceId.ToString()).c_str()))
								provider.MoveEntityScriptInstance(instance.InstanceId, true);
							ImGui::SameLine();
							if (ImGui::Button(("Down##" + instance.InstanceId.ToString()).c_str()))
								provider.MoveEntityScriptInstance(instance.InstanceId, false);
							ImGui::SameLine();
							if (ImGui::Button(("Remove##" + instance.InstanceId.ToString()).c_str())) {
								provider.RemoveEntityScriptInstance(instance.InstanceId);
								ImGui::TreePop();
								break;
							}
							ImGui::Separator();
							for (const auto& fieldEntry : instance.Fields) {
								const std::string& fieldName = fieldEntry.first;
								const ScriptFieldValue& fieldValue = fieldEntry.second;
								ScriptFieldValue updated = fieldValue;
								bool changed = false;
								const std::string widgetLabel = fieldName + "##" + instance.InstanceId.ToString();
								switch (fieldValue.Type) {
								case ScriptFieldType::Bool: {
									bool v = fieldValue.BoolValue;
									if (ImGui::Checkbox(widgetLabel.c_str(), &v)) {
										updated.BoolValue = v;
										changed = true;
									}
									break;
								}
								case ScriptFieldType::Int: {
									int v = fieldValue.IntValue;
									if (ImGui::DragInt(widgetLabel.c_str(), &v, 1.0f)) {
										updated.IntValue = v;
										changed = true;
									}
									break;
								}
								case ScriptFieldType::Float: {
									float v = fieldValue.FloatValue;
									if (ImGui::DragFloat(widgetLabel.c_str(), &v, 0.05f)) {
										updated.FloatValue = v;
										changed = true;
									}
									break;
								}
								case ScriptFieldType::Vec2: {
									glm::vec2 v = fieldValue.Vec2Value;
									if (ImGui::DragFloat2(widgetLabel.c_str(), glm::value_ptr(v), 0.05f)) {
										updated.Vec2Value = v;
										changed = true;
									}
									break;
								}
								case ScriptFieldType::Vec3: {
									glm::vec3 v = fieldValue.Vec3Value;
									if (ImGui::DragFloat3(widgetLabel.c_str(), glm::value_ptr(v), 0.05f)) {
										updated.Vec3Value = v;
										changed = true;
									}
									break;
								}
								case ScriptFieldType::Vec4: {
									glm::vec4 v = fieldValue.Vec4Value;
									if (ImGui::DragFloat4(widgetLabel.c_str(), glm::value_ptr(v), 0.05f)) {
										updated.Vec4Value = v;
										changed = true;
									}
									break;
								}
								case ScriptFieldType::Entity: {
									uint64_t u = fieldValue.EntityUUIDValue;
									if (ImGui::DragScalar(widgetLabel.c_str(), ImGuiDataType_U64, &u, 1.0f)) {
										updated.EntityUUIDValue = u;
										changed = true;
									}
									break;
								}
								case ScriptFieldType::String: {
									char textBuf[256];
									strncpy_s(textBuf, fieldValue.StringValue.c_str(), sizeof(textBuf) - 1);
									textBuf[sizeof(textBuf) - 1] = '\0';
									if (ImGui::InputText(widgetLabel.c_str(), textBuf, sizeof(textBuf))) {
										updated.StringValue = textBuf;
										changed = true;
									}
									break;
								}
								default:
									break;
								}

								if (changed)
									provider.SetEntityScriptField(instance.InstanceId, fieldName, updated);
							}
							ImGui::TreePop();
						}
						if (i + 1 < d.ScriptInstances.size())
							ImGui::Separator();
					}
				}
			}
			if (!snapshot.AllowComponentEdit)
				ImGui::EndDisabled();
			break;
		}
		}

		ImGui::End();
	}

} // namespace Ehu
