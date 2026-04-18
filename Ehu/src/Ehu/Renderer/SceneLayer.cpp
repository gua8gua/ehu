#include "ehupch.h"
#include "SceneLayer.h"
#include "Scene/Scene.h"
#include "ECS/World.h"
#include "ECS/Components.h"
#include "RenderQueue.h"
#include "Renderer/Camera/Camera.h"
#include "Renderer/Text/Font.h"
#include "Platform/Render/Resources/Texture2D.h"
#include "Project/Project.h"
#include "Core/Application.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Ehu {

	namespace {

		bool MatchesRenderChannel(World& world, Entity entity, RenderChannelId channel) {
			if (const RenderFilterComponent* filter = world.GetComponent<RenderFilterComponent>(entity))
				return filter->Channel == channel;
			return channel == BuiltinRenderChannels::Default;
		}

		static std::unordered_map<std::string, Texture2D*> s_TextureCache;
		static std::unordered_map<std::string, Font*> s_FontCache;

		Texture2D* ResolveTexture(const std::string& relativePath) {
			if (relativePath.empty())
				return nullptr;
			Ref<Project> proj = Project::GetActive();
			const std::string absPath = proj ? proj->GetAssetFileSystemPath(relativePath) : relativePath;
			auto it = s_TextureCache.find(absPath);
			if (it != s_TextureCache.end())
				return it->second;
			Texture2D* tex = Texture2D::CreateFromFile(absPath);
			s_TextureCache[absPath] = tex;
			return tex;
		}

		Font* ResolveFont(const std::string& relativePath, float pixelHeight) {
			if (relativePath.empty())
				return nullptr;
			Ref<Project> proj = Project::GetActive();
			const std::string absPath = proj ? proj->GetAssetFileSystemPath(relativePath) : relativePath;
			const std::string key = absPath + "|" + std::to_string(static_cast<int>(pixelHeight));
			auto it = s_FontCache.find(key);
			if (it != s_FontCache.end())
				return it->second;
			Font* font = Font::CreateFromFile(absPath, pixelHeight);
			if (!font)
				font = Font::CreatePlaceholder(pixelHeight);
			s_FontCache[key] = font;
			return font;
		}

		void RunCameraSync(World& w) {
			w.Each<TransformComponent, CameraComponent>([&](Entity, TransformComponent& t, CameraComponent& c) {
				if (!c.Camera) return;
				Camera* cam = c.Camera;
				const glm::vec3 pos = t.Position;
				const glm::vec3 eulerDeg = glm::degrees(glm::eulerAngles(t.Rotation));
				if (auto* pc = dynamic_cast<PerspectiveCamera*>(cam)) {
					pc->SetPosition(pos);
					pc->SetRotation(eulerDeg);
				} else if (auto* oc = dynamic_cast<OrthographicCamera*>(cam)) {
					oc->SetPosition(pos);
					oc->SetRotation(eulerDeg.z);
				}
			});
		}

		void SubmitEntitiesOfLayerToQueue(World& world, RenderChannelId channel, RenderQueue& queue, Camera* viewCamera) {
			world.Each<TransformComponent, SpriteComponent>([&](Entity entity, TransformComponent& t, SpriteComponent& s) {
				if (!MatchesRenderChannel(world, entity, channel)) return;
				const glm::mat4& wt = t.GetWorldMatrix();
				glm::vec3 pos(wt[3][0], wt[3][1], wt[3][2]);
				if (Texture2D* tex = ResolveTexture(s.TexturePath))
					queue.SubmitTexturedQuad(wt, tex, s.TilingFactor, s.Color, s.SortKey, s.Transparent, viewCamera, 0, entity.id);
				else
					queue.SubmitQuad(pos, s.Size, s.Color, s.SortKey, s.Transparent, viewCamera, 0, entity.id);
			});
			world.Each<TransformComponent, CircleRendererComponent>([&](Entity entity, TransformComponent& t, CircleRendererComponent& c) {
				if (!MatchesRenderChannel(world, entity, channel)) return;
				const glm::mat4& wt = t.GetWorldMatrix();
				queue.SubmitCircle(wt, c.Color, c.Thickness, c.Fade, c.SortKey, c.Transparent, viewCamera, 0, entity.id);
			});
			world.Each<TransformComponent, TextComponent>([&](Entity entity, TransformComponent& t, TextComponent& tx) {
				if (!MatchesRenderChannel(world, entity, channel)) return;
				if (tx.TextString.empty()) return;
				Font* font = ResolveFont(tx.FontPath, tx.PixelHeight);
				if (!font) return;
				const glm::mat4& wt = t.GetWorldMatrix();
				glm::vec3 pos(wt[3][0], wt[3][1], wt[3][2]);
				queue.SubmitText(pos, tx.TextString, font, tx.Color, tx.SortKey, tx.Transparent, viewCamera, 0, entity.id);
			});
			world.Each<TransformComponent, MeshComponent>([&](Entity entity, TransformComponent& t, MeshComponent& m) {
				if (!MatchesRenderChannel(world, entity, channel)) return;
				if (!m.VAO || m.IndexCount == 0) return;
				queue.SubmitMesh(m.VAO, m.IndexCount, t.GetWorldMatrix(), m.Color, m.SortKey, m.Transparent, viewCamera, 0);
			});
		}

	} // namespace

	SceneLayer::SceneLayer(const std::string& name, RenderChannelId channel)
		: Layer(name), m_RenderChannel(channel)
	{}

	SceneLayer::~SceneLayer() = default;

	void SceneLayer::OnAttach() {
		Layer::OnAttach();
	}

	void SceneLayer::OnUpdate(const TimeStep& timestep) {
		for (Scene* scene : Application::Get().GetActivatedScenes())
			scene->OnUpdate(timestep);
		OnUpdateScene(timestep);
	}

	void SceneLayer::SubmitTo(RenderQueue& queue) const {
		SubmitTo(queue, nullptr);
	}

	void SceneLayer::SubmitTo(RenderQueue& queue, Camera* viewCameraOverride) const {
		Camera* viewCam = viewCameraOverride;
		for (Scene* scene : Application::Get().GetActivatedScenes()) {
			if (!scene) continue;
			World& world = scene->GetWorld();
			RunCameraSync(world);
			if (!viewCam)
				viewCam = scene->GetMainCamera();
			if (!viewCam) continue;
			SubmitEntitiesOfLayerToQueue(world, m_RenderChannel, queue, viewCam);
			if (viewCameraOverride) viewCam = viewCameraOverride;
		}
	}

} // namespace Ehu
