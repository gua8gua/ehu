#include "ehupch.h"
#include "SceneSerializer.h"
#include "Scene.h"
#include "ECS/Components.h"
#include "Core/FileSystem.h"
#include "Core/UUID.h"
#include "Core/Ref.h"
#include "Renderer/Camera/Camera.h"
#include <glm/glm.hpp>
#include <sstream>
#include <algorithm>

namespace Ehu {

	namespace {

		static const char* SCENE_MAGIC = "EhuScene";
		static const int SCENE_VERSION = 1;

		void WriteVec2(std::ostringstream& out, const glm::vec2& v) {
			out << v.x << ' ' << v.y;
		}
		void WriteVec3(std::ostringstream& out, const glm::vec3& v) {
			out << v.x << ' ' << v.y << ' ' << v.z;
		}
		void WriteVec4(std::ostringstream& out, const glm::vec4& v) {
			out << v.x << ' ' << v.y << ' ' << v.z << ' ' << v.w;
		}
		void WriteQuat(std::ostringstream& out, const glm::quat& q) {
			out << q.x << ' ' << q.y << ' ' << q.z << ' ' << q.w;
		}

	}

	bool SceneSerializer::Serialize(Scene* scene, const std::string& path) {
		if (!scene) return false;
		std::ostringstream out;
		out << SCENE_MAGIC << ' ' << SCENE_VERSION << '\n';

		UUID mainCameraId;
		if (scene->GetWorld().IsValid(scene->GetMainCameraEntity())) {
			const IdComponent* idc = scene->GetWorld().GetComponent<IdComponent>(scene->GetMainCameraEntity());
			if (idc) mainCameraId = idc->Id;
		}
		out << "MainCamera " << mainCameraId.ToString() << '\n';
		out << "---\n";

		std::vector<Entity> entities = scene->GetEntities();
		for (Entity e : entities) {
			const IdComponent* idc = scene->GetWorld().GetComponent<IdComponent>(e);
			const TagComponent* tc = scene->GetWorld().GetComponent<TagComponent>(e);
			const TransformComponent* tr = scene->GetWorld().GetComponent<TransformComponent>(e);
			const SpriteComponent* sp = scene->GetWorld().GetComponent<SpriteComponent>(e);
			const CameraComponent* cc = scene->GetWorld().GetComponent<CameraComponent>(e);
			if (!idc) continue;

			out << "Entity\n";
			out << "Id " << idc->Id.ToString() << '\n';
			if (tc) out << "Tag " << tc->Name << '\n';
			if (tr) {
				out << "Transform ";
				WriteVec3(out, tr->Position); out << ' ';
				WriteVec3(out, tr->Scale); out << ' ';
				WriteQuat(out, tr->Rotation); out << '\n';
			}
			if (sp) {
				out << "Sprite ";
				WriteVec2(out, sp->Size); out << ' ';
				WriteVec4(out, sp->Color); out << ' ';
				out << sp->SortKey << ' ' << (sp->Transparent ? 1 : 0) << '\n';
			}
			if (cc && cc->Camera) {
				if (auto* oc = dynamic_cast<OrthographicCamera*>(cc->Camera)) {
					out << "Camera Ortho " << oc->GetLeft() << ' ' << oc->GetRight() << ' ' << oc->GetBottom() << ' '
						<< oc->GetTop() << ' ' << oc->GetNearZ() << ' ' << oc->GetFarZ() << '\n';
				} else if (auto* pc = dynamic_cast<PerspectiveCamera*>(cc->Camera)) {
					out << "Camera Persp " << pc->GetFov() << ' ' << pc->GetAspect() << ' ' << pc->GetNearZ() << ' ' << pc->GetFarZ() << '\n';
				}
			}
			out << "EndEntity\n";
		}

		return FileSystem::WriteTextFile(path, out.str());
	}

	bool SceneSerializer::Deserialize(Scene* scene, const std::string& path) {
		if (!scene) return false;
		std::string content = FileSystem::ReadTextFile(path);
		if (content.empty()) return false;

		std::istringstream in(content);
		std::string line;
		if (!std::getline(in, line) || line.find(SCENE_MAGIC) != 0) return false;

		UUID mainCameraId(0);
		if (std::getline(in, line)) {
			if (line.size() > 11 && line.substr(0, 11) == "MainCamera ") {
				mainCameraId = UUID::FromString(line.substr(11));
			}
		}
		while (std::getline(in, line) && line != "---") {}

		// 清空当前实体与场景持有的相机（主相机引用会在后面重设）
		scene->ClearOwnedCameras();
		std::vector<Entity> toDestroy = scene->GetEntities();
		for (Entity e : toDestroy)
			scene->DestroyEntity(e);

		while (std::getline(in, line)) {
			if (line != "Entity") continue;

			Entity e = scene->CreateEntity();
			IdComponent* idc = scene->GetWorld().GetComponent<IdComponent>(e);
			TagComponent* tc = scene->GetWorld().GetComponent<TagComponent>(e);
			if (!idc) continue;

			while (std::getline(in, line) && line != "EndEntity") {
				if (line.size() < 4) continue;
				if (line.compare(0, 3, "Id ") == 0) {
					UUID u = UUID::FromString(line.substr(3));
					if (u.Raw() != 0) idc->Id = u;
				} else if (line.compare(0, 4, "Tag ") == 0) {
					if (tc) tc->Name = line.substr(4);
				} else if (line.compare(0, 10, "Transform ") == 0) {
					TransformComponent tr;
					float px, py, pz, sx_f, sy_f, sz_f, qx_f, qy_f, qz_f, qw_f;
					if (sscanf(line.c_str() + 10, "%f %f %f %f %f %f %f %f %f %f",
						&px, &py, &pz, &sx_f, &sy_f, &sz_f, &qx_f, &qy_f, &qz_f, &qw_f) == 10) {
						tr.Position = glm::vec3(px, py, pz);
						tr.Scale = glm::vec3(sx_f, sy_f, sz_f);
						tr.Rotation = glm::quat(qw_f, qx_f, qy_f, qz_f);
						tr.Dirty = true;
						scene->GetWorld().AddComponent(e, tr);
					}
				} else if (line.compare(0, 6, "Sprite ") == 0) {
					SpriteComponent sp;
					int trans = 0;
					if (sscanf(line.c_str() + 6, "%f %f %f %f %f %f %f %d",
						&sp.Size.x, &sp.Size.y, &sp.Color.r, &sp.Color.g, &sp.Color.b, &sp.Color.a,
						&sp.SortKey, &trans) >= 2) {
						sp.Transparent = (trans != 0);
						scene->GetWorld().AddComponent(e, sp);
					}
				} else if (line.compare(0, 12, "Camera Ortho ") == 0) {
					float l, r, b, t, n, f;
					if (sscanf(line.c_str() + 12, "%f %f %f %f %f %f", &l, &r, &b, &t, &n, &f) == 6) {
						Camera* cam = scene->AddOwnedCamera(CreateScope<OrthographicCamera>(l, r, b, t, n, f));
						scene->GetWorld().AddComponent(e, CameraComponent{ cam });
					}
				} else if (line.compare(0, 12, "Camera Persp ") == 0) {
					float fov, aspect, n, f;
					if (sscanf(line.c_str() + 12, "%f %f %f %f", &fov, &aspect, &n, &f) == 4) {
						Camera* cam = scene->AddOwnedCamera(CreateScope<PerspectiveCamera>(fov, aspect, n, f));
						scene->GetWorld().AddComponent(e, CameraComponent{ cam });
					}
				}
			}
		}

		if (mainCameraId.Raw() != 0) {
			Entity camEntity = scene->FindEntityByUUID(mainCameraId);
			if (scene->GetWorld().IsValid(camEntity))
				scene->SetMainCamera(camEntity);
		}
		return true;
	}

} // namespace Ehu
