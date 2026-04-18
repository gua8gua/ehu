#pragma once

#include "Core/Core.h"
#include "Components.h"
#include <string>
#include <vector>
#include <utility>

namespace Ehu {

	class EHU_API LayerRegistry {
	public:
		// 重置为内置层
		static void ResetToBuiltins();

		// 渲染层相关
		static RenderChannelId RegisterRenderChannel(const std::string& name);
		static RenderChannelId RegisterRenderChannelWithId(const std::string& name, RenderChannelId id);
		static bool TryGetRenderChannelId(const std::string& name, RenderChannelId& outId);
		static std::string GetRenderChannelName(RenderChannelId id);
		static std::vector<std::pair<RenderChannelId, std::string>> GetRenderChannels();

		// 碰撞层相关
		static uint32_t RegisterCollisionLayer(const std::string& name);
		static uint32_t RegisterCollisionLayerWithBit(const std::string& name, uint32_t bit, uint32_t defaultMask = 0xFFFFFFFFu);
		static bool TryGetCollisionLayerBit(const std::string& name, uint32_t& outBit);
		static std::string GetCollisionLayerName(uint32_t bit);
		static std::vector<std::pair<uint32_t, std::string>> GetCollisionLayers();
		static void SetCollisionDefaultMask(uint32_t bit, uint32_t mask);
		static uint32_t GetCollisionDefaultMask(uint32_t bit);
		static std::vector<std::pair<uint32_t, uint32_t>> GetCollisionDefaultMasks();
	};

} // namespace Ehu
