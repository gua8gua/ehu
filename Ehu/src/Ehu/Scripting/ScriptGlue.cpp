#include "ScriptGlue.h"
#include "Core/Application.h"
#include "Core/Log.h"
#include "ScriptEngine.h"
#include "Platform/IO/Input.h"
#include "Renderer/Camera/Camera.h"
#include "Scene/Scene.h"
#include "ECS/World.h"
#include "ECS/Components.h"
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#ifdef EHU_ENABLE_MONO
#include <mono/jit/jit.h>
#include <mono/metadata/object.h>
#endif

namespace Ehu {

#ifdef EHU_ENABLE_MONO
	namespace {
		std::string MonoStringToUtf8(MonoString* managedString) {
			if (!managedString)
				return {};
			char* utf8 = mono_string_to_utf8(managedString);
			if (!utf8)
				return {};
			std::string result = utf8;
			mono_free(utf8);
			return result;
		}

		void InternalCall_Log(MonoString* message) {
			if (!message) {
				EHU_CORE_INFO("[C#] <null>");
				return;
			}
			char* utf8 = mono_string_to_utf8(message);
			if (!utf8) {
				EHU_CORE_INFO("[C#] <invalid>");
				return;
			}
			EHU_CORE_INFO("[C#] {}", utf8);
			mono_free(utf8);
		}

		float InternalCall_GetDeltaTime() {
			return Application::Get().GetDeltaTime();
		}

		uint64_t InternalCall_GetCurrentEntityId() {
			Entity entity = ScriptEngine::GetInvocationEntity();
			return static_cast<uint64_t>(entity.id);
		}

		void InternalCall_GetPosition(float* x, float* y, float* z) {
			Scene* scene = ScriptEngine::GetInvocationScene();
			Entity entity = ScriptEngine::GetInvocationEntity();
			if (!scene || !scene->GetWorld().IsValid(entity))
				return;
			TransformComponent* transform = scene->GetWorld().GetComponent<TransformComponent>(entity);
			if (!transform)
				return;
			if (x) *x = transform->Position.x;
			if (y) *y = transform->Position.y;
			if (z) *z = transform->Position.z;
		}

		void InternalCall_SetPosition(float x, float y, float z) {
			Scene* scene = ScriptEngine::GetInvocationScene();
			Entity entity = ScriptEngine::GetInvocationEntity();
			if (!scene || !scene->GetWorld().IsValid(entity))
				return;
			TransformComponent* transform = scene->GetWorld().GetComponent<TransformComponent>(entity);
			if (!transform)
				return;
			transform->SetPosition(x, y, z);
		}

		void InternalCall_GetScale(float* x, float* y, float* z) {
			Scene* scene = ScriptEngine::GetInvocationScene();
			Entity entity = ScriptEngine::GetInvocationEntity();
			if (!scene || !scene->GetWorld().IsValid(entity))
				return;
			TransformComponent* transform = scene->GetWorld().GetComponent<TransformComponent>(entity);
			if (!transform)
				return;
			if (x) *x = transform->Scale.x;
			if (y) *y = transform->Scale.y;
			if (z) *z = transform->Scale.z;
		}

		void InternalCall_SetScale(float x, float y, float z) {
			Scene* scene = ScriptEngine::GetInvocationScene();
			Entity entity = ScriptEngine::GetInvocationEntity();
			if (!scene || !scene->GetWorld().IsValid(entity))
				return;
			TransformComponent* transform = scene->GetWorld().GetComponent<TransformComponent>(entity);
			if (!transform)
				return;
			transform->SetScale(glm::vec3(x, y, z));
		}

		void InternalCall_GetRotationEuler(float* xDeg, float* yDeg, float* zDeg) {
			Scene* scene = ScriptEngine::GetInvocationScene();
			Entity entity = ScriptEngine::GetInvocationEntity();
			if (!scene || !scene->GetWorld().IsValid(entity))
				return;
			TransformComponent* transform = scene->GetWorld().GetComponent<TransformComponent>(entity);
			if (!transform)
				return;
			const glm::vec3 eulerDeg = glm::degrees(glm::eulerAngles(transform->Rotation));
			if (xDeg) *xDeg = eulerDeg.x;
			if (yDeg) *yDeg = eulerDeg.y;
			if (zDeg) *zDeg = eulerDeg.z;
		}

		void InternalCall_SetRotationEuler(float xDeg, float yDeg, float zDeg) {
			Scene* scene = ScriptEngine::GetInvocationScene();
			Entity entity = ScriptEngine::GetInvocationEntity();
			if (!scene || !scene->GetWorld().IsValid(entity))
				return;
			TransformComponent* transform = scene->GetWorld().GetComponent<TransformComponent>(entity);
			if (!transform)
				return;
			transform->SetRotation(glm::radians(glm::vec3(xDeg, yDeg, zDeg)));
		}

		bool InternalCall_HasTransform() {
			Scene* scene = ScriptEngine::GetInvocationScene();
			Entity entity = ScriptEngine::GetInvocationEntity();
			if (!scene || !scene->GetWorld().IsValid(entity))
				return false;
			return scene->GetWorld().HasComponent<TransformComponent>(entity);
		}

		uint64_t InternalCall_GetEntityUuid() {
			Scene* scene = ScriptEngine::GetInvocationScene();
			Entity entity = ScriptEngine::GetInvocationEntity();
			if (!scene || !scene->GetWorld().IsValid(entity))
				return 0;
			const IdComponent* id = scene->GetWorld().GetComponent<IdComponent>(entity);
			return id ? static_cast<uint64_t>(id->Id.Raw()) : 0ull;
		}

		MonoString* InternalCall_GetTag() {
			MonoDomain* domain = ScriptEngine::GetDomain();
			if (!domain)
				return nullptr;
			Scene* scene = ScriptEngine::GetInvocationScene();
			Entity entity = ScriptEngine::GetInvocationEntity();
			if (!scene || !scene->GetWorld().IsValid(entity))
				return mono_string_new(domain, "");
			const TagComponent* tag = scene->GetWorld().GetComponent<TagComponent>(entity);
			const char* text = (tag && !tag->Name.empty()) ? tag->Name.c_str() : "";
			return mono_string_new(domain, text);
		}

		void InternalCall_SetTag(MonoString* name) {
			Scene* scene = ScriptEngine::GetInvocationScene();
			Entity entity = ScriptEngine::GetInvocationEntity();
			if (!scene || !scene->GetWorld().IsValid(entity))
				return;
			World& world = scene->GetWorld();
			TagComponent* tag = world.GetComponent<TagComponent>(entity);
			if (!tag) {
				world.AddComponent(entity, TagComponent{});
				tag = world.GetComponent<TagComponent>(entity);
			}
			if (tag)
				tag->Name = MonoStringToUtf8(name);
		}

		bool InternalCall_IsKeyPressed(int32_t keyCode) {
			return Input::IsKeyPressed(static_cast<KeyCode>(keyCode));
		}

		bool InternalCall_IsMouseButtonPressed(int32_t button) {
			return Input::IsMouseButtonPressed(static_cast<MouseCode>(button));
		}

		void InternalCall_GetMousePosition(float* x, float* y) {
			auto pos = Input::GetMousePosition();
			if (x)
				*x = pos.first;
			if (y)
				*y = pos.second;
		}

		float InternalCall_GetFixedDeltaTime() {
			return Application::Get().GetFixedDeltaTime();
		}
	}
#endif

	void ScriptGlue::RegisterInternalCalls() {
#ifdef EHU_ENABLE_MONO
		mono_add_internal_call("Ehu.InternalCalls::Log_Native", reinterpret_cast<const void*>(InternalCall_Log));
		mono_add_internal_call("Ehu.InternalCalls::GetDeltaTime_Native", reinterpret_cast<const void*>(InternalCall_GetDeltaTime));
		mono_add_internal_call("Ehu.InternalCalls::GetCurrentEntityId_Native", reinterpret_cast<const void*>(InternalCall_GetCurrentEntityId));
		mono_add_internal_call("Ehu.InternalCalls::GetPosition_Native", reinterpret_cast<const void*>(InternalCall_GetPosition));
		mono_add_internal_call("Ehu.InternalCalls::SetPosition_Native", reinterpret_cast<const void*>(InternalCall_SetPosition));
		mono_add_internal_call("Ehu.InternalCalls::GetScale_Native", reinterpret_cast<const void*>(InternalCall_GetScale));
		mono_add_internal_call("Ehu.InternalCalls::SetScale_Native", reinterpret_cast<const void*>(InternalCall_SetScale));
		mono_add_internal_call("Ehu.InternalCalls::GetRotationEuler_Native", reinterpret_cast<const void*>(InternalCall_GetRotationEuler));
		mono_add_internal_call("Ehu.InternalCalls::SetRotationEuler_Native", reinterpret_cast<const void*>(InternalCall_SetRotationEuler));
		mono_add_internal_call("Ehu.InternalCalls::HasTransform_Native", reinterpret_cast<const void*>(InternalCall_HasTransform));
		mono_add_internal_call("Ehu.InternalCalls::GetEntityUuid_Native", reinterpret_cast<const void*>(InternalCall_GetEntityUuid));
		mono_add_internal_call("Ehu.InternalCalls::GetTag_Native", reinterpret_cast<const void*>(InternalCall_GetTag));
		mono_add_internal_call("Ehu.InternalCalls::SetTag_Native", reinterpret_cast<const void*>(InternalCall_SetTag));
		mono_add_internal_call("Ehu.InternalCalls::IsKeyPressed_Native", reinterpret_cast<const void*>(InternalCall_IsKeyPressed));
		mono_add_internal_call("Ehu.InternalCalls::IsMouseButtonPressed_Native", reinterpret_cast<const void*>(InternalCall_IsMouseButtonPressed));
		mono_add_internal_call("Ehu.InternalCalls::GetMousePosition_Native", reinterpret_cast<const void*>(InternalCall_GetMousePosition));
		mono_add_internal_call("Ehu.InternalCalls::GetFixedDeltaTime_Native", reinterpret_cast<const void*>(InternalCall_GetFixedDeltaTime));
#endif
	}

} // namespace Ehu

