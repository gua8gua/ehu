#pragma once

#include "Core/Core.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"
#include <functional>
#include <string>
#include <vector>

namespace Ehu {

	class Project;
	class Scene;

	struct EHU_API AssetCreateContext {
		Project& TargetProject;
		std::string RelativeDirectory;
		std::string Name;
		std::string ErrorMessage;
	};

	struct EHU_API EntityCreateContext {
		Scene* TargetScene = nullptr;
		std::string Name;
		RenderChannelId RenderChannel = BuiltinRenderChannels::Default;
		uint32_t CollisionLayer = 1u;
		uint32_t CollisionMask = 0xFFFFFFFFu;
		Entity CreatedEntity{};
		std::string ErrorMessage;
	};

	struct EHU_API AssetCreatorEntry {
		std::string Id;
		std::string DisplayName;
		std::string Category;
		std::function<bool(AssetCreateContext&)> CreateFn;
	};

	struct EHU_API EntityCreatorEntry {
		std::string Id;
		std::string DisplayName;
		std::string Category;
		std::function<bool(EntityCreateContext&)> CreateFn;
	};

	class EHU_API CreationRegistry {
	public:
		static void EnsureDefaultCreatorsRegistered();

		static void RegisterAssetCreator(const AssetCreatorEntry& entry);
		static void RegisterEntityCreator(const EntityCreatorEntry& entry);

		static std::vector<AssetCreatorEntry> GetAssetCreators();
		static std::vector<EntityCreatorEntry> GetEntityCreators();

		static bool CreateAssetById(const std::string& id, AssetCreateContext& context);
		static bool CreateEntityById(const std::string& id, EntityCreateContext& context);
	};

} // namespace Ehu
