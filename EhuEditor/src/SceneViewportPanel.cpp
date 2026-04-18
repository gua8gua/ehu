#include "SceneViewportPanel.h"
#include "EditorSession.h"
#include "Core/Application.h"
#include "Core/KeyCodes.h"
#include "Editor/EditorContext.h"
#include "Editor/EditorPanelData.h"
#include "Editor/EditorContext.h"
#include "Editor/ViewportRenderer.h"
#include "EditorUIState.h"
#include "ECS/Components.h"
#include "Scene/Scene.h"
#include "Renderer/Camera/EditorCamera.h"
#include "Platform/IO/Input.h"
#include "Core/FileSystem.h"
#include "ImGui/ImGuiLayer.h"
#include "Project/Project.h"
#include "Platform/Render/Resources/Texture2D.h"
#include "Core/FileSystem.h"
#include <filesystem>
#include <imgui.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Ehu {

	namespace {

		static bool PointInTriangle(int px, int py, int x1, int y1, int x2, int y2, int x3, int y3) {
			auto sign = [](int p1x, int p1y, int p2x, int p2y, int p3x, int p3y) -> int {
				return (p1x - p3x) * (p2y - p3y) - (p2x - p3x) * (p1y - p3y);
			};
			const int d1 = sign(px, py, x1, y1, x2, y2);
			const int d2 = sign(px, py, x2, y2, x3, y3);
			const int d3 = sign(px, py, x3, y3, x1, y1);
			const bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
			const bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
			return !(hasNeg && hasPos);
		}

		static void FillGrayBackground(std::vector<uint8_t>& rgba, int S) {
			for (int i = 0; i < S * S * 4; i += 4) {
				rgba[i] = 40;
				rgba[i + 1] = 42;
				rgba[i + 2] = 48;
				rgba[i + 3] = 255;
			}
		}

		static void SetPx(std::vector<uint8_t>& rgba, int S, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
			if (x < 0 || y < 0 || x >= S || y >= S)
				return;
			const size_t i = (static_cast<size_t>(y) * S + x) * 4;
			rgba[i] = r;
			rgba[i + 1] = g;
			rgba[i + 2] = b;
			rgba[i + 3] = a;
		}

		static Texture2D* TryLoadToolbarIcon(const char* fileName) {
			const std::string path = FileSystem::Join(FileSystem::Join("Resources", "Icons"), fileName);
			if (!FileSystem::Exists(path))
				return nullptr;
			Texture2D* t = Texture2D::CreateFromFile(path);
			if (!t || t->GetRendererID() == 0) {
				delete t;
				return nullptr;
			}
			return t;
		}

		static Texture2D* MakePlayIcon() {
			const int S = 32;
			std::vector<uint8_t> rgba(S * S * 4);
			FillGrayBackground(rgba, S);
			for (int y = 0; y < S; ++y)
				for (int x = 0; x < S; ++x)
					if (PointInTriangle(x, y, 10, 10, 10, 22, 22, 16))
						SetPx(rgba, S, x, y, 90, 220, 110);
			return Texture2D::Create(S, S, rgba.data());
		}

		static Texture2D* MakeStopIcon() {
			const int S = 32;
			std::vector<uint8_t> rgba(S * S * 4);
			FillGrayBackground(rgba, S);
			for (int y = 10; y <= 22; ++y)
				for (int x = 10; x <= 22; ++x)
					SetPx(rgba, S, x, y, 240, 70, 70);
			return Texture2D::Create(S, S, rgba.data());
		}

		static Texture2D* MakeSimulateIcon() {
			const int S = 32;
			std::vector<uint8_t> rgba(S * S * 4);
			FillGrayBackground(rgba, S);
			const int cx = 16, cy = 16, R = 9;
			for (int y = 0; y < S; ++y)
				for (int x = 0; x < S; ++x) {
					const int dx = x - cx, dy = y - cy;
					const int d2 = dx * dx + dy * dy;
					if (d2 >= R * R - 6 && d2 <= R * R + 6)
						SetPx(rgba, S, x, y, 100, 180, 255);
				}
			return Texture2D::Create(S, S, rgba.data());
		}

		static Texture2D* MakePauseIcon() {
			const int S = 32;
			std::vector<uint8_t> rgba(S * S * 4);
			FillGrayBackground(rgba, S);
			for (int y = 8; y < 24; ++y) {
				for (int x = 9; x < 14; ++x)
					SetPx(rgba, S, x, y, 240, 240, 245);
				for (int x = 18; x < 23; ++x)
					SetPx(rgba, S, x, y, 240, 240, 245);
			}
			return Texture2D::Create(S, S, rgba.data());
		}

		static Texture2D* MakeStepIcon() {
			const int S = 32;
			std::vector<uint8_t> rgba(S * S * 4);
			FillGrayBackground(rgba, S);
			for (int y = 8; y < 24; ++y)
				for (int x = 9; x < 12; ++x)
					SetPx(rgba, S, x, y, 220, 220, 230);
			for (int y = 0; y < S; ++y)
				for (int x = 0; x < S; ++x)
					if (PointInTriangle(x, y, 14, 10, 14, 22, 24, 16))
						SetPx(rgba, S, x, y, 90, 220, 110);
			return Texture2D::Create(S, S, rgba.data());
		}

		static bool ToolbarIconButton(const char* id, Texture2D* tex, const char* textFallback) {
			constexpr float kBtn = 22.0f;
			if (tex && tex->GetRendererID()) {
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));
				const bool pressed = ImGui::ImageButton(id, ImTextureRef((ImTextureID)(uintptr_t)tex->GetRendererID()), ImVec2(kBtn, kBtn));
				ImGui::PopStyleVar();
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					ImGui::SetTooltip("%s", textFallback);
				return pressed;
			}
			if (ImGui::Button(textFallback))
				return true;
			return false;
		}

		bool ProjectWorldToScreen(const glm::vec3& worldPosition, Camera* camera, const ImVec2& imageMin, const ImVec2& imageSize, ImVec2& outScreen, float& outDepth) {
			if (!camera || imageSize.x <= 0.0f || imageSize.y <= 0.0f)
				return false;

			glm::vec4 clip = camera->GetViewProjectionMatrix() * glm::vec4(worldPosition, 1.0f);
			if (clip.w <= 0.0f)
				return false;

			glm::vec3 ndc = glm::vec3(clip) / clip.w;
			if (ndc.z < -1.0f || ndc.z > 1.0f)
				return false;

			outScreen.x = imageMin.x + ((ndc.x * 0.5f) + 0.5f) * imageSize.x;
			outScreen.y = imageMin.y + ((-ndc.y * 0.5f) + 0.5f) * imageSize.y;
			outDepth = ndc.z;
			return true;
		}

		glm::vec3 ScreenToWorldAtDepth(const ImVec2& mousePosition, float ndcDepth, Camera* camera, const ImVec2& imageMin, const ImVec2& imageSize) {
			const float x = ((mousePosition.x - imageMin.x) / imageSize.x) * 2.0f - 1.0f;
			const float y = -((((mousePosition.y - imageMin.y) / imageSize.y) * 2.0f) - 1.0f);
			glm::vec4 clip(x, y, ndcDepth, 1.0f);
			glm::mat4 invViewProj = glm::inverse(camera->GetViewProjectionMatrix());
			glm::vec4 world = invViewProj * clip;
			if (world.w != 0.0f)
				world /= world.w;
			return glm::vec3(world);
		}

		bool IsSceneAssetPath(const std::string& relativePath) {
			return FileSystem::GetExtension(relativePath) == ".ehuscene";
		}

		Entity PickEntity(Scene* scene, Camera* camera, const ImVec2& imageMin, const ImVec2& imageSize, const ImVec2& mousePosition) {
			if (!scene || !camera)
				return {};

			Entity bestEntity{};
			float bestScore = 1000000.0f;
			World& world = scene->GetWorld();
			for (Entity entity : scene->GetEntities()) {
				TransformComponent* transform = world.GetComponent<TransformComponent>(entity);
				if (!transform)
					continue;

				ImVec2 center{};
				float depth = 0.0f;
				if (!ProjectWorldToScreen(transform->Position, camera, imageMin, imageSize, center, depth))
					continue;

				float pickRadius = 14.0f;
				if (SpriteComponent* sprite = world.GetComponent<SpriteComponent>(entity))
					pickRadius = 8.0f + (sprite->Size.x + sprite->Size.y) * 6.0f;

				const float dx = mousePosition.x - center.x;
				const float dy = mousePosition.y - center.y;
				const float distanceSq = dx * dx + dy * dy;
				if (distanceSq > pickRadius * pickRadius)
					continue;

				const float score = distanceSq + (depth + 1.0f) * 10.0f;
				if (score < bestScore) {
					bestScore = score;
					bestEntity = entity;
				}
			}

			return bestEntity;
		}

	} // namespace

	SceneViewportPanel::SceneViewportPanel() {
		m_Viewport = new ViewportRenderer();
	}

	SceneViewportPanel::~SceneViewportPanel() {
		ReleaseToolbarIcons();
		delete m_Viewport;
	}

	void SceneViewportPanel::ReleaseToolbarIcons() {
		auto release = [](Texture2D*& t) {
			delete t;
			t = nullptr;
		};
		release(m_IconPlay);
		release(m_IconPause);
		release(m_IconSimulate);
		release(m_IconStep);
		release(m_IconStop);
		m_ToolbarIconsReady = false;
	}

	void SceneViewportPanel::EnsureToolbarIcons() {
		if (m_ToolbarIconsReady)
			return;
		m_ToolbarIconsReady = true;
		m_IconPlay = TryLoadToolbarIcon("PlayButton.png");
		if (!m_IconPlay)
			m_IconPlay = MakePlayIcon();
		m_IconPause = TryLoadToolbarIcon("PauseButton.png");
		if (!m_IconPause)
			m_IconPause = MakePauseIcon();
		m_IconSimulate = TryLoadToolbarIcon("SimulateButton.png");
		if (!m_IconSimulate)
			m_IconSimulate = MakeSimulateIcon();
		m_IconStep = TryLoadToolbarIcon("StepButton.png");
		if (!m_IconStep)
			m_IconStep = MakeStepIcon();
		m_IconStop = TryLoadToolbarIcon("StopButton.png");
		if (!m_IconStop)
			m_IconStop = MakeStopIcon();
	}

	void SceneViewportPanel::DrawToolbar(const EditorSession& session, SceneViewportPanelResult& result) {
		EnsureToolbarIcons();
		const bool playing = session.IsPlaying();
		const bool simulating = session.IsSimulating();
		const bool inRuntime = playing || simulating;

		if (!inRuntime) {
			if (ToolbarIconButton("vp_play", m_IconPlay, "Play"))
				result.RequestPlay = true;
			ImGui::SameLine();
			if (ToolbarIconButton("vp_sim", m_IconSimulate, "Simulate"))
				result.RequestSimulate = true;
		} else {
			if (ToolbarIconButton("vp_stop", m_IconStop, "Stop"))
				result.RequestStop = true;
			ImGui::SameLine();
			ImGui::TextDisabled("%s", playing ? "Playing" : "Simulating");
			ImGui::SameLine();
			Scene* active = session.GetActiveScene();
			if (active) {
				if (ToolbarIconButton("vp_pause", m_IconPause, active->IsPaused() ? "Resume" : "Pause"))
					result.RequestPauseToggle = true;
				ImGui::SameLine();
				if (ToolbarIconButton("vp_step", m_IconStep, "Step"))
					result.RequestStep = true;
				ImGui::SameLine();
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Q"))
			m_GizmoMode = GizmoMode::None;
		ImGui::SameLine();
		if (ImGui::Button("W"))
			m_GizmoMode = GizmoMode::Translate;
		ImGui::SameLine();
		if (ImGui::Button("E"))
			m_GizmoMode = GizmoMode::Rotate;
		ImGui::SameLine();
		if (ImGui::Button("R"))
			m_GizmoMode = GizmoMode::Scale;
	}

	SceneViewportPanelResult SceneViewportPanel::OnImGuiRender(EditorSession& session, bool* pOpen, EditorUIState* uiState) {
		SceneViewportPanelResult result;
		if (pOpen && !*pOpen)
			return result;
		Application& app = Application::Get();
		Scene* scene = session.GetActiveScene();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		if (!ImGui::Begin("Scene", pOpen, ImGuiWindowFlags_NoScrollbar)) {
			ImGui::End();
			ImGui::PopStyleVar();
			return result;
		}
		ImGui::PopStyleVar();

		DrawToolbar(session, result);
		ImGui::Separator();

		m_Focused = ImGui::IsWindowFocused();
		m_Hovered = ImGui::IsWindowHovered();
		if (ImGuiLayer* imgui = app.GetImGuiLayer())
			imgui->BlockEvents(!m_Hovered);

		ImVec2 size = ImGui::GetContentRegionAvail();
		if (size.x <= 0 || size.y <= 0) {
			ImGui::End();
			return result;
		}

		m_Viewport->SetSize(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));
		{
			Scene* activeScene = session.GetActiveScene();
			Entity selected{};
			if (EditorContext::Get().HasEntitySelection() && EditorContext::Get().GetSelectedScene() == activeScene)
				selected = EditorContext::Get().GetSelectedEntity();
			const bool showPhysics = uiState ? uiState->ShowPhysicsColliders : false;
			const bool showOutline = uiState ? uiState->ShowSelectionOutline : true;
			m_Viewport->Render(app, activeScene, showPhysics, showOutline, selected);
		}

		uint32_t texId = m_Viewport->GetColorAttachmentTextureID(0);
		const ImVec2 imageMin = ImGui::GetCursorScreenPos();
		const ImVec2 imageMax(imageMin.x + size.x, imageMin.y + size.y);
		if (texId != 0) {
			ImGui::Image((ImTextureID)(uintptr_t)texId, size, ImVec2(0, 1), ImVec2(1, 0));
		} else {
			ImGui::TextUnformatted("Scene viewport unavailable.");
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET")) {
				if (payload->Data && payload->DataSize > 0) {
					const char* raw = static_cast<const char*>(payload->Data);
					std::string relativePath(raw, raw + payload->DataSize - 1);
					if (IsSceneAssetPath(relativePath))
						result.OpenSceneRelativePath = relativePath;
				}
			} else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
				if (payload->Data && payload->DataSize >= sizeof(wchar_t)) {
					const wchar_t* wpath = static_cast<const wchar_t*>(payload->Data);
					std::filesystem::path absPath(wpath);
					Ref<Project> proj = session.GetProject();
					if (proj) {
						std::filesystem::path base(proj->GetAssetDirectory());
						std::error_code ec;
						std::filesystem::path rel = std::filesystem::relative(absPath, base, ec);
						if (!ec) {
							std::string relStr = rel.generic_string();
							if (IsSceneAssetPath(relStr))
								result.OpenSceneRelativePath = relStr;
						}
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

		const bool allowViewportEdit = session.IsEditing();
		if (m_Focused && m_Viewport->GetEditorCamera() && allowViewportEdit) {
			EditorCamera* cam = m_Viewport->GetEditorCamera();
			float delta = 0.05f;
			if (Input::IsKeyPressed(Key::Left))
				cam->SetRotationDelta(0.0f, delta);
			if (Input::IsKeyPressed(Key::Right))
				cam->SetRotationDelta(0.0f, -delta);
			if (Input::IsKeyPressed(Key::Up))
				cam->SetRotationDelta(-delta, 0.0f);
			if (Input::IsKeyPressed(Key::Down))
				cam->SetRotationDelta(delta, 0.0f);
			cam->OnUpdate();
		}

		Camera* viewCamera = m_Viewport->GetEditorCamera();

		if (allowViewportEdit && scene && viewCamera && ImGui::IsItemHovered()) {
			const ImVec2 mousePosition = ImGui::GetMousePos();
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !m_GizmoActive) {
				const float lx = mousePosition.x - imageMin.x;
				const float ly = mousePosition.y - imageMin.y;
				Entity picked{};
				if (lx >= 0.0f && ly >= 0.0f && lx < size.x && ly < size.y && m_Viewport->GetWidth() > 0 && m_Viewport->GetHeight() > 0) {
					const uint32_t px = static_cast<uint32_t>(lx);
					const uint32_t py = m_Viewport->GetHeight() - 1u - static_cast<uint32_t>(ly);
					const uint32_t id = m_Viewport->ReadEntityIdAt(px, py);
					if (id != 0)
						picked = scene->FindEntityByEntityId(id);
				}
				if (!scene->GetWorld().IsValid(picked))
					picked = PickEntity(scene, viewCamera, imageMin, size, mousePosition);
				if (scene->GetWorld().IsValid(picked))
					EditorPanelDataProvider::Get().SelectEntity(scene, picked);
				else
					EditorContext::Get().ClearSelectedEntity();
			}
		}

		if (allowViewportEdit && scene && viewCamera && EditorContext::Get().HasEntitySelection() && EditorContext::Get().GetSelectedScene() == scene) {
			Entity selected = EditorContext::Get().GetSelectedEntity();
			TransformComponent* transform = scene->GetWorld().GetComponent<TransformComponent>(selected);
			if (transform) {
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				ImVec2 center{};
				float depth = 0.0f;
				if (ProjectWorldToScreen(transform->Position, viewCamera, imageMin, size, center, depth)) {
					drawList->AddCircle(center, 8.0f, IM_COL32(255, 230, 0, 255), 0, 2.0f);

					const ImVec2 mousePosition = ImGui::GetMousePos();
					const bool mouseHeld = ImGui::IsMouseDown(ImGuiMouseButton_Left);
					const bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
					const float dx = mousePosition.x - center.x;
					const float dy = mousePosition.y - center.y;
					const float dist = sqrtf(dx * dx + dy * dy);

					if (m_GizmoMode == GizmoMode::Translate) {
						drawList->AddLine(ImVec2(center.x - 20.0f, center.y), ImVec2(center.x + 20.0f, center.y), IM_COL32(255, 80, 80, 255), 2.0f);
						drawList->AddLine(ImVec2(center.x, center.y - 20.0f), ImVec2(center.x, center.y + 20.0f), IM_COL32(80, 255, 80, 255), 2.0f);
						if (mouseClicked && dist <= 14.0f) {
							m_GizmoActive = true;
							m_LastMouseX = mousePosition.x;
							m_LastMouseY = mousePosition.y;
						}
						if (m_GizmoActive && mouseHeld) {
							ImVec2 previousMouse(m_LastMouseX, m_LastMouseY);
							glm::vec3 prevWorld = ScreenToWorldAtDepth(previousMouse, depth, viewCamera, imageMin, size);
							glm::vec3 currWorld = ScreenToWorldAtDepth(mousePosition, depth, viewCamera, imageMin, size);
							transform->SetPosition(transform->Position + (currWorld - prevWorld));
							m_LastMouseX = mousePosition.x;
							m_LastMouseY = mousePosition.y;
						}
					} else if (m_GizmoMode == GizmoMode::Rotate) {
						drawList->AddCircle(center, 28.0f, IM_COL32(80, 180, 255, 255), 0, 2.0f);
						if (mouseClicked && dist >= 22.0f && dist <= 34.0f) {
							m_GizmoActive = true;
							m_LastRotateAngle = atan2f(dy, dx);
						}
						if (m_GizmoActive && mouseHeld) {
							const float angle = atan2f(dy, dx);
							const float deltaAngle = angle - m_LastRotateAngle;
							glm::vec3 euler = glm::degrees(glm::eulerAngles(transform->Rotation));
							euler.z += glm::degrees(deltaAngle);
							transform->SetRotation(glm::radians(euler));
							m_LastRotateAngle = angle;
						}
					} else if (m_GizmoMode == GizmoMode::Scale) {
						const ImVec2 scaleHandle(center.x + 24.0f, center.y - 24.0f);
						drawList->AddRectFilled(ImVec2(scaleHandle.x - 5.0f, scaleHandle.y - 5.0f), ImVec2(scaleHandle.x + 5.0f, scaleHandle.y + 5.0f), IM_COL32(120, 220, 255, 255));
						const float hx = mousePosition.x - scaleHandle.x;
						const float hy = mousePosition.y - scaleHandle.y;
						if (mouseClicked && (hx * hx + hy * hy) <= 100.0f) {
							m_GizmoActive = true;
							m_LastMouseX = mousePosition.x;
							m_LastMouseY = mousePosition.y;
						}
						if (m_GizmoActive && mouseHeld) {
							const float mouseDelta = (mousePosition.x - m_LastMouseX) - (mousePosition.y - m_LastMouseY);
							glm::vec3 sc = transform->Scale + glm::vec3(mouseDelta * 0.01f);
							sc.x = std::max(0.01f, sc.x);
							sc.y = std::max(0.01f, sc.y);
							sc.z = std::max(0.01f, sc.z);
							transform->SetScale(sc);
							m_LastMouseX = mousePosition.x;
							m_LastMouseY = mousePosition.y;
						}
					}

					if (m_GizmoActive && !mouseHeld)
						m_GizmoActive = false;
				}
			}
		} else {
			m_GizmoActive = false;
		}

		ImGui::End();
		return result;
	}

} // namespace Ehu
