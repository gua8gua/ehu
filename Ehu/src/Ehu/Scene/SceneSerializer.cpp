#include "ehupch.h"
#include "SceneSerializer.h"
#include "Scene.h"
#include "ECS/Components.h"
#include "ECS/LayerRegistry.h"
#include "Core/FileSystem.h"
#include "Core/Log.h"
#include "Core/UUID.h"
#include "Core/Ref.h"
#include "Renderer/Camera/Camera.h"
#include <glm/glm.hpp>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cctype>

namespace Ehu {

	namespace {

		static const char* SCENE_MAGIC = "EhuScene";
		static const int SCENE_VERSION = 5;

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

		std::string RenderChannelToToken(RenderChannelId channel) {
			return LayerRegistry::GetRenderChannelName(channel);
		}

		RenderChannelId RenderChannelFromToken(const std::string& token) {
			if (token.empty())
				return BuiltinRenderChannels::Default;
			const bool numeric = std::all_of(token.begin(), token.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
			if (numeric)
				return static_cast<RenderChannelId>(std::stoul(token));

			RenderChannelId id = BuiltinRenderChannels::Default;
			if (LayerRegistry::TryGetRenderChannelId(token, id))
				return id;
			return LayerRegistry::RegisterRenderChannel(token);
		}

		const char* ScriptFieldTypeToToken(ScriptFieldType type) {
			switch (type) {
			case ScriptFieldType::Bool: return "Bool";
			case ScriptFieldType::Int: return "Int";
			case ScriptFieldType::Float: return "Float";
			case ScriptFieldType::Vec3: return "Vec3";
			case ScriptFieldType::String: return "String";
			default: return "None";
			}
		}

		ScriptFieldType ScriptFieldTypeFromToken(const std::string& token) {
			if (token == "Bool") return ScriptFieldType::Bool;
			if (token == "Int") return ScriptFieldType::Int;
			if (token == "Float") return ScriptFieldType::Float;
			if (token == "Vec3") return ScriptFieldType::Vec3;
			if (token == "String") return ScriptFieldType::String;
			return ScriptFieldType::None;
		}

		void WriteScriptField(std::ostringstream& out, const std::string& name, const ScriptFieldValue& value) {
			out << "ScriptField " << std::quoted(name) << ' ' << ScriptFieldTypeToToken(value.Type);
			switch (value.Type) {
			case ScriptFieldType::Bool:
				out << ' ' << (value.BoolValue ? 1 : 0);
				break;
			case ScriptFieldType::Int:
				out << ' ' << value.IntValue;
				break;
			case ScriptFieldType::Float:
				out << ' ' << value.FloatValue;
				break;
			case ScriptFieldType::Vec3:
				out << ' ' << value.Vec3Value.x << ' ' << value.Vec3Value.y << ' ' << value.Vec3Value.z;
				break;
			case ScriptFieldType::String:
				out << ' ' << std::quoted(value.StringValue);
				break;
			default:
				break;
			}
			out << '\n';
		}

		void WriteScriptInstance(std::ostringstream& out, const ScriptInstanceData& instance) {
			out << "ScriptInstance " << instance.InstanceId.ToString() << ' ' << std::quoted(instance.ClassName) << '\n';
			for (const auto& [fieldName, fieldValue] : instance.Fields)
				WriteScriptField(out, fieldName, fieldValue);
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
			const ScriptComponent* sc = scene->GetWorld().GetComponent<ScriptComponent>(e);
			const RenderFilterComponent* rf = scene->GetWorld().GetComponent<RenderFilterComponent>(e);
			const PhysicsFilterComponent* pf = scene->GetWorld().GetComponent<PhysicsFilterComponent>(e);
			const Rigidbody2DComponent* rb = scene->GetWorld().GetComponent<Rigidbody2DComponent>(e);
			const BoxCollider2DComponent* box = scene->GetWorld().GetComponent<BoxCollider2DComponent>(e);
			const CircleCollider2DComponent* circle = scene->GetWorld().GetComponent<CircleCollider2DComponent>(e);
			const CircleRendererComponent* circleR = scene->GetWorld().GetComponent<CircleRendererComponent>(e);
			const TextComponent* textC = scene->GetWorld().GetComponent<TextComponent>(e);
			if (!idc) continue;

			out << "Entity\n";
			out << "Id " << idc->Id.ToString() << '\n';
			if (tc) {
				out << "Tag " << std::quoted(tc->Name) << '\n';
			}
			if (rf)
				out << "RenderChannel " << std::quoted(RenderChannelToToken(rf->Channel)) << '\n';
			if (pf)
				out << "PhysicsFilter " << pf->CollisionLayer << ' ' << pf->CollisionMask << '\n';
			if (rb)
				out << "Rigidbody2D " << static_cast<int>(rb->Type) << ' ' << rb->LinearVelocity.x << ' ' << rb->LinearVelocity.y << ' ' << rb->GravityScale << ' ' << (rb->FixedRotation ? 1 : 0) << '\n';
			if (box)
				out << "BoxCollider2D " << box->Offset.x << ' ' << box->Offset.y << ' ' << box->Size.x << ' ' << box->Size.y << ' '
					<< box->Density << ' ' << box->Friction << ' ' << box->Restitution << ' ' << box->RestitutionThreshold << '\n';
			if (circle)
				out << "CircleCollider2D " << circle->Offset.x << ' ' << circle->Offset.y << ' ' << circle->Radius << ' '
					<< circle->Density << ' ' << circle->Friction << ' ' << circle->Restitution << ' ' << circle->RestitutionThreshold << '\n';
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
				if (!sp->TexturePath.empty())
					out << "SpriteTexture " << std::quoted(sp->TexturePath) << ' ' << sp->TilingFactor << '\n';
			}
			if (circleR) {
				out << "CircleRenderer ";
				WriteVec4(out, circleR->Color); out << ' ' << circleR->Thickness << ' ' << circleR->Fade << ' '
					<< circleR->SortKey << ' ' << (circleR->Transparent ? 1 : 0) << '\n';
			}
			if (textC) {
				out << "Text " << std::quoted(textC->TextString) << ' ' << std::quoted(textC->FontPath) << ' '
					<< textC->PixelHeight << ' ' << textC->Kerning << ' ' << textC->LineSpacing << ' ';
				WriteVec4(out, textC->Color); out << ' ' << textC->SortKey << ' ' << (textC->Transparent ? 1 : 0) << '\n';
			}
			if (cc && cc->Camera) {
				if (auto* oc = dynamic_cast<OrthographicCamera*>(cc->Camera)) {
					out << "Camera Ortho " << oc->GetLeft() << ' ' << oc->GetRight() << ' ' << oc->GetBottom() << ' '
						<< oc->GetTop() << ' ' << oc->GetNearZ() << ' ' << oc->GetFarZ() << ' '
						<< (cc->Primary ? 1 : 0) << ' ' << (cc->FixedAspectRatio ? 1 : 0) << '\n';
				} else if (auto* pc = dynamic_cast<PerspectiveCamera*>(cc->Camera)) {
					out << "Camera Persp " << pc->GetFov() << ' ' << pc->GetAspect() << ' ' << pc->GetNearZ() << ' ' << pc->GetFarZ() << ' '
						<< (cc->Primary ? 1 : 0) << ' ' << (cc->FixedAspectRatio ? 1 : 0) << '\n';
				}
			}
			if (sc) {
				for (const ScriptInstanceData& instance : sc->Instances) {
					if (instance.ClassName.empty())
						continue;
					WriteScriptInstance(out, instance);
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
		scene->m_EntityMap.clear();

		while (std::getline(in, line)) {
			if (line != "Entity") continue;

			Entity e = scene->CreateEntity();
			IdComponent* idc = scene->GetWorld().GetComponent<IdComponent>(e);
			TagComponent* tc = scene->GetWorld().GetComponent<TagComponent>(e);
			if (!idc) continue;

			ScriptInstanceData* currentScriptInstance = nullptr;
			while (std::getline(in, line) && line != "EndEntity") {
				if (line.size() < 4) continue;
				if (line.compare(0, 3, "Id ") == 0) {
					const UUID previousId = idc->Id;
					UUID u = UUID::FromString(line.substr(3));
					if (u.Raw() != 0) {
						scene->m_EntityMap.erase(previousId.Raw());
						idc->Id = u;
						scene->m_EntityMap[idc->Id.Raw()] = e;
					}
				} else if (line.compare(0, 4, "Tag ") == 0) {
					if (tc) {
						std::istringstream tagStream(line.substr(4));
						if (!(tagStream >> std::quoted(tc->Name)))
							tc->Name = line.substr(4);
					}
				} else if (line.compare(0, 14, "RenderChannel ") == 0) {
					RenderFilterComponent filter;
					std::string token;
					std::istringstream channelStream(line.substr(14));
					if (!(channelStream >> std::quoted(token))) {
						token = line.substr(14);
						const size_t begin = token.find_first_not_of(' ');
						if (begin == std::string::npos)
							token.clear();
						else {
							const size_t endPos = token.find_last_not_of(' ');
							token = token.substr(begin, endPos - begin + 1);
						}
					}
					filter.Channel = RenderChannelFromToken(token);
					scene->GetWorld().AddComponent(e, filter);
				} else if (line.compare(0, 14, "PhysicsFilter ") == 0) {
					PhysicsFilterComponent filter;
					if (sscanf(line.c_str() + 14, "%u %u", &filter.CollisionLayer, &filter.CollisionMask) == 2)
						scene->GetWorld().AddComponent(e, filter);
				} else if (line.compare(0, 12, "Rigidbody2D ") == 0) {
					int type = 0;
					Rigidbody2DComponent body;
					int fixedRotation = 1;
					if (sscanf(line.c_str() + 12, "%d %f %f %f %d", &type, &body.LinearVelocity.x, &body.LinearVelocity.y, &body.GravityScale, &fixedRotation) == 5) {
						body.Type = static_cast<Rigidbody2DBodyType>(type);
						body.FixedRotation = (fixedRotation != 0);
						scene->GetWorld().AddComponent(e, body);
					}
				} else if (line.compare(0, 14, "BoxCollider2D ") == 0) {
					BoxCollider2DComponent collider;
					if (sscanf(line.c_str() + 14, "%f %f %f %f %f %f %f %f",
						&collider.Offset.x, &collider.Offset.y, &collider.Size.x, &collider.Size.y,
						&collider.Density, &collider.Friction, &collider.Restitution, &collider.RestitutionThreshold) == 8) {
						scene->GetWorld().AddComponent(e, collider);
					} else if (sscanf(line.c_str() + 14, "%f %f %f %f %f %f %f",
						&collider.Offset.x, &collider.Offset.y, &collider.Size.x, &collider.Size.y,
						&collider.Density, &collider.Friction, &collider.Restitution) == 7) {
						collider.RestitutionThreshold = 0.5f;
						scene->GetWorld().AddComponent(e, collider);
					}
				} else if (line.compare(0, 18, "CircleCollider2D ") == 0) {
					CircleCollider2DComponent collider;
					if (sscanf(line.c_str() + 18, "%f %f %f %f %f %f %f",
						&collider.Offset.x, &collider.Offset.y, &collider.Radius,
						&collider.Density, &collider.Friction, &collider.Restitution, &collider.RestitutionThreshold) == 7) {
						scene->GetWorld().AddComponent(e, collider);
					}
				} else if (line.compare(0, 12, "RenderLayer ") == 0) {
					// 兼容旧格式：旧字段绑定 Layer 名称，现迁移为默认渲染通道
					EHU_CORE_WARN("[SceneSerializer] Legacy RenderLayer key detected, remapped to RenderChannel=Default");
					scene->GetWorld().AddComponent(e, RenderFilterComponent{ BuiltinRenderChannels::Default });
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
					int primary = 1, fixedA = 0;
					if (sscanf(line.c_str() + 12, "%f %f %f %f %f %f %d %d", &l, &r, &b, &t, &n, &f, &primary, &fixedA) == 8) {
						Camera* cam = scene->AddOwnedCamera(CreateScope<OrthographicCamera>(l, r, b, t, n, f));
						CameraComponent cc;
						cc.Camera = cam;
						cc.Primary = (primary != 0);
						cc.FixedAspectRatio = (fixedA != 0);
						scene->GetWorld().AddComponent(e, cc);
					} else if (sscanf(line.c_str() + 12, "%f %f %f %f %f %f", &l, &r, &b, &t, &n, &f) == 6) {
						Camera* cam = scene->AddOwnedCamera(CreateScope<OrthographicCamera>(l, r, b, t, n, f));
						scene->GetWorld().AddComponent(e, CameraComponent{ cam, true, false });
					}
				} else if (line.compare(0, 12, "Camera Persp ") == 0) {
					float fov, aspect, nearZ, farZ;
					int primary = 1, fixedA = 0;
					if (sscanf(line.c_str() + 12, "%f %f %f %f %d %d", &fov, &aspect, &nearZ, &farZ, &primary, &fixedA) == 6) {
						Camera* cam = scene->AddOwnedCamera(CreateScope<PerspectiveCamera>(fov, aspect, nearZ, farZ));
						CameraComponent cc;
						cc.Camera = cam;
						cc.Primary = (primary != 0);
						cc.FixedAspectRatio = (fixedA != 0);
						scene->GetWorld().AddComponent(e, cc);
					} else if (sscanf(line.c_str() + 12, "%f %f %f %f", &fov, &aspect, &nearZ, &farZ) == 4) {
						Camera* cam = scene->AddOwnedCamera(CreateScope<PerspectiveCamera>(fov, aspect, nearZ, farZ));
						scene->GetWorld().AddComponent(e, CameraComponent{ cam, true, false });
					}
				} else if (line.compare(0, 14, "SpriteTexture ") == 0) {
					std::istringstream st(line.substr(14));
					std::string path;
					float tiling = 1.0f;
					if (st >> std::quoted(path) >> tiling) {
						if (SpriteComponent* sp = scene->GetWorld().GetComponent<SpriteComponent>(e)) {
							sp->TexturePath = path;
							sp->TilingFactor = tiling;
						}
					}
				} else if (line.compare(0, 16, "CircleRenderer ") == 0) {
					CircleRendererComponent cr;
					int trans = 0;
					if (sscanf(line.c_str() + 16, "%f %f %f %f %f %f %f %d",
						&cr.Color.r, &cr.Color.g, &cr.Color.b, &cr.Color.a,
						&cr.Thickness, &cr.Fade, &cr.SortKey, &trans) == 8) {
						cr.Transparent = (trans != 0);
						scene->GetWorld().AddComponent(e, cr);
					}
				} else if (line.compare(0, 5, "Text ") == 0) {
					std::istringstream st(line.substr(5));
					TextComponent tx;
					int trans = 0;
					if (st >> std::quoted(tx.TextString) >> std::quoted(tx.FontPath) >> tx.PixelHeight >> tx.Kerning >> tx.LineSpacing
						>> tx.Color.r >> tx.Color.g >> tx.Color.b >> tx.Color.a >> tx.SortKey >> trans) {
						tx.Transparent = (trans != 0);
						scene->GetWorld().AddComponent(e, tx);
					}
				} else if (line.compare(0, 15, "ScriptInstance ") == 0) {
					ScriptComponent* scriptComponent = scene->GetWorld().GetComponent<ScriptComponent>(e);
					if (!scriptComponent) {
						scene->GetWorld().AddComponent(e, ScriptComponent{});
						scriptComponent = scene->GetWorld().GetComponent<ScriptComponent>(e);
					}
					if (!scriptComponent)
						continue;

					std::istringstream scriptStream(line.substr(15));
					std::string idToken;
					std::string className;
					if (!(scriptStream >> idToken >> std::quoted(className)))
						continue;
					ScriptInstanceData instance;
					instance.InstanceId = UUID::FromString(idToken);
					if (instance.InstanceId.Raw() == 0)
						instance.InstanceId = UUID();
					instance.ClassName = className;
					scriptComponent->Instances.push_back(std::move(instance));
					currentScriptInstance = &scriptComponent->Instances.back();
				} else if (line.compare(0, 7, "Script ") == 0) {
					// 兼容旧格式：单脚本记录
					ScriptComponent* scriptComponent = scene->GetWorld().GetComponent<ScriptComponent>(e);
					if (!scriptComponent) {
						scene->GetWorld().AddComponent(e, ScriptComponent{});
						scriptComponent = scene->GetWorld().GetComponent<ScriptComponent>(e);
					}
					if (!scriptComponent)
						continue;
					ScriptInstanceData instance;
					instance.InstanceId = UUID();
					instance.ClassName = line.substr(7);
					scriptComponent->Instances.push_back(std::move(instance));
					currentScriptInstance = &scriptComponent->Instances.back();
				} else if (line.compare(0, 12, "ScriptField ") == 0) {
					if (!currentScriptInstance)
						continue;

					std::istringstream fieldStream(line.substr(12));
					std::string fieldName;
					std::string typeToken;
					if (!(fieldStream >> std::quoted(fieldName) >> typeToken))
						continue;

					ScriptFieldValue value;
					value.Type = ScriptFieldTypeFromToken(typeToken);
					switch (value.Type) {
					case ScriptFieldType::Bool: {
						int raw = 0;
						if (fieldStream >> raw)
							value.BoolValue = (raw != 0);
						break;
					}
					case ScriptFieldType::Int:
						fieldStream >> value.IntValue;
						break;
					case ScriptFieldType::Float:
						fieldStream >> value.FloatValue;
						break;
					case ScriptFieldType::Vec3:
						fieldStream >> value.Vec3Value.x >> value.Vec3Value.y >> value.Vec3Value.z;
						break;
					case ScriptFieldType::String:
						fieldStream >> std::quoted(value.StringValue);
						break;
					default:
						break;
					}
					currentScriptInstance->Fields[fieldName] = value;
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
