#include "ehupch.h"
#include "ComponentReflection.h"
#include "World.h"
#include "Components.h"

namespace Ehu {

	namespace {

		template<typename T>
		void CloneComponent(const World& srcWorld, Entity srcEntity, World& dstWorld, Entity dstEntity) {
			const T* src = srcWorld.GetComponent<T>(srcEntity);
			if (!src)
				return;
			dstWorld.AddComponent(dstEntity, *src);
		}

		void CloneRigidbody2D(const World& srcWorld, Entity srcEntity, World& dstWorld, Entity dstEntity) {
			const Rigidbody2DComponent* src = srcWorld.GetComponent<Rigidbody2DComponent>(srcEntity);
			if (!src) return;
			Rigidbody2DComponent copy = *src;
			copy.RuntimeBody = nullptr;
			dstWorld.AddComponent(dstEntity, copy);
		}

		void CloneBoxCollider2D(const World& srcWorld, Entity srcEntity, World& dstWorld, Entity dstEntity) {
			const BoxCollider2DComponent* src = srcWorld.GetComponent<BoxCollider2DComponent>(srcEntity);
			if (!src) return;
			BoxCollider2DComponent copy = *src;
			copy.RuntimeFixture = nullptr;
			dstWorld.AddComponent(dstEntity, copy);
		}

		void CloneCircleCollider2D(const World& srcWorld, Entity srcEntity, World& dstWorld, Entity dstEntity) {
			const CircleCollider2DComponent* src = srcWorld.GetComponent<CircleCollider2DComponent>(srcEntity);
			if (!src) return;
			CircleCollider2DComponent copy = *src;
			copy.RuntimeFixture = nullptr;
			dstWorld.AddComponent(dstEntity, copy);
		}

		std::vector<ComponentCloneDescriptor> BuildCloneDescriptors() {
			std::vector<ComponentCloneDescriptor> descriptors;
			descriptors.push_back({ "TransformComponent", GetTypeId<TransformComponent>(), &CloneComponent<TransformComponent> });
			descriptors.push_back({ "SpriteComponent", GetTypeId<SpriteComponent>(), &CloneComponent<SpriteComponent> });
			descriptors.push_back({ "MeshComponent", GetTypeId<MeshComponent>(), &CloneComponent<MeshComponent> });
			descriptors.push_back({ "RenderFilterComponent", GetTypeId<RenderFilterComponent>(), &CloneComponent<RenderFilterComponent> });
			descriptors.push_back({ "PhysicsFilterComponent", GetTypeId<PhysicsFilterComponent>(), &CloneComponent<PhysicsFilterComponent> });
			descriptors.push_back({ "Rigidbody2DComponent", GetTypeId<Rigidbody2DComponent>(), &CloneRigidbody2D });
			descriptors.push_back({ "BoxCollider2DComponent", GetTypeId<BoxCollider2DComponent>(), &CloneBoxCollider2D });
			descriptors.push_back({ "CircleCollider2DComponent", GetTypeId<CircleCollider2DComponent>(), &CloneCircleCollider2D });
			descriptors.push_back({ "CircleRendererComponent", GetTypeId<CircleRendererComponent>(), &CloneComponent<CircleRendererComponent> });
			descriptors.push_back({ "TextComponent", GetTypeId<TextComponent>(), &CloneComponent<TextComponent> });
			descriptors.push_back({ "ScriptComponent", GetTypeId<ScriptComponent>(), &CloneComponent<ScriptComponent> });
			return descriptors;
		}

	}

	const std::vector<ComponentCloneDescriptor>& ComponentReflectionRegistry::GetCloneDescriptors() {
		static const std::vector<ComponentCloneDescriptor> s_Descriptors = BuildCloneDescriptors();
		return s_Descriptors;
	}

} // namespace Ehu
