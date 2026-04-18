#include "ehupch.h"
#include "LayerRegistry.h"
#include <unordered_map>
#include <mutex>
#include <algorithm>

namespace Ehu {

	namespace {
		// 注册状态
		struct RegistryState {
			// 互斥锁
			std::mutex Mutex;

			// 渲染层相关
			std::unordered_map<std::string, RenderChannelId> RenderByName;
			std::unordered_map<RenderChannelId, std::string> RenderById;
			RenderChannelId NextRenderChannelId = 3u;

			// 碰撞层相关
			std::unordered_map<std::string, uint32_t> CollisionByName;
			std::unordered_map<uint32_t, std::string> CollisionByBit;
			std::unordered_map<uint32_t, uint32_t> CollisionDefaultMaskByBit;
			uint32_t NextCollisionBitIndex = 3u;
		};

		void FillBuiltins(RegistryState& state) {
			state.RenderByName.clear();
			state.RenderById.clear();
			state.CollisionByName.clear();
			state.CollisionByBit.clear();
			state.CollisionDefaultMaskByBit.clear();

			state.RenderByName["Default"] = BuiltinRenderChannels::Default;
			state.RenderByName["UI"] = BuiltinRenderChannels::UI;
			state.RenderByName["Debug"] = BuiltinRenderChannels::Debug;
			state.RenderById[BuiltinRenderChannels::Default] = "Default";
			state.RenderById[BuiltinRenderChannels::UI] = "UI";
			state.RenderById[BuiltinRenderChannels::Debug] = "Debug";
			state.NextRenderChannelId = 3u;

			state.CollisionByName["Default"] = 1u << 0;
			state.CollisionByName["Trigger"] = 1u << 1;
			state.CollisionByName["UI"] = 1u << 2;
			state.CollisionByBit[1u << 0] = "Default";
			state.CollisionByBit[1u << 1] = "Trigger";
			state.CollisionByBit[1u << 2] = "UI";
			state.CollisionDefaultMaskByBit[1u << 0] = 0xFFFFFFFFu;
			state.CollisionDefaultMaskByBit[1u << 1] = 0xFFFFFFFFu;
			state.CollisionDefaultMaskByBit[1u << 2] = 0xFFFFFFFFu;
			state.NextCollisionBitIndex = 3u;
		}

		RegistryState& GetState() {
			static RegistryState* s_State = [] {
				RegistryState* state = new RegistryState();
				FillBuiltins(*state);
				return state;
			}();
			return *s_State;
		}
	}

	void LayerRegistry::ResetToBuiltins() {
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		FillBuiltins(state);
	}

	RenderChannelId LayerRegistry::RegisterRenderChannel(const std::string& name) {
		if (name.empty())
			return BuiltinRenderChannels::Default;
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		auto it = state.RenderByName.find(name);
		if (it != state.RenderByName.end())
			return it->second;
		const RenderChannelId id = state.NextRenderChannelId++;
		state.RenderByName[name] = id;
		state.RenderById[id] = name;
		return id;
	}

	RenderChannelId LayerRegistry::RegisterRenderChannelWithId(const std::string& name, RenderChannelId id) {
		if (name.empty())
			return BuiltinRenderChannels::Default;
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		state.RenderByName[name] = id;
		state.RenderById[id] = name;
		if (id >= state.NextRenderChannelId)
			state.NextRenderChannelId = id + 1;
		return id;
	}

	bool LayerRegistry::TryGetRenderChannelId(const std::string& name, RenderChannelId& outId) {
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		auto it = state.RenderByName.find(name);
		if (it == state.RenderByName.end())
			return false;
		outId = it->second;
		return true;
	}

	std::string LayerRegistry::GetRenderChannelName(RenderChannelId id) {
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		auto it = state.RenderById.find(id);
		if (it != state.RenderById.end())
			return it->second;
		return "RenderChannel_" + std::to_string(id);
	}

	std::vector<std::pair<RenderChannelId, std::string>> LayerRegistry::GetRenderChannels() {
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		std::vector<std::pair<RenderChannelId, std::string>> result;
		result.reserve(state.RenderById.size());
		for (const auto& [id, name] : state.RenderById)
			result.emplace_back(id, name);
		std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
		return result;
	}

	uint32_t LayerRegistry::RegisterCollisionLayer(const std::string& name) {
		if (name.empty())
			return 1u;
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		auto it = state.CollisionByName.find(name);
		if (it != state.CollisionByName.end())
			return it->second;
		if (state.NextCollisionBitIndex >= 31u)
			return 0u;
		const uint32_t bit = 1u << state.NextCollisionBitIndex++;
		state.CollisionByName[name] = bit;
		state.CollisionByBit[bit] = name;
		state.CollisionDefaultMaskByBit[bit] = 0xFFFFFFFFu;
		return bit;
	}

	uint32_t LayerRegistry::RegisterCollisionLayerWithBit(const std::string& name, uint32_t bit, uint32_t defaultMask) {
		if (name.empty() || bit == 0u)
			return 0u;
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		state.CollisionByName[name] = bit;
		state.CollisionByBit[bit] = name;
		state.CollisionDefaultMaskByBit[bit] = defaultMask;
		uint32_t bitIndex = 0;
		while (((1u << bitIndex) != bit) && bitIndex < 31u)
			++bitIndex;
		if (bitIndex + 1 > state.NextCollisionBitIndex)
			state.NextCollisionBitIndex = bitIndex + 1;
		return bit;
	}

	bool LayerRegistry::TryGetCollisionLayerBit(const std::string& name, uint32_t& outBit) {
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		auto it = state.CollisionByName.find(name);
		if (it == state.CollisionByName.end())
			return false;
		outBit = it->second;
		return true;
	}

	std::string LayerRegistry::GetCollisionLayerName(uint32_t bit) {
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		auto it = state.CollisionByBit.find(bit);
		if (it != state.CollisionByBit.end())
			return it->second;
		return "CollisionLayer_" + std::to_string(bit);
	}

	std::vector<std::pair<uint32_t, std::string>> LayerRegistry::GetCollisionLayers() {
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		std::vector<std::pair<uint32_t, std::string>> result;
		result.reserve(state.CollisionByBit.size());
		for (const auto& [bit, name] : state.CollisionByBit)
			result.emplace_back(bit, name);
		std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
		return result;
	}

	void LayerRegistry::SetCollisionDefaultMask(uint32_t bit, uint32_t mask) {
		if (bit == 0u)
			return;
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		state.CollisionDefaultMaskByBit[bit] = mask;
	}

	uint32_t LayerRegistry::GetCollisionDefaultMask(uint32_t bit) {
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		auto it = state.CollisionDefaultMaskByBit.find(bit);
		return it != state.CollisionDefaultMaskByBit.end() ? it->second : 0xFFFFFFFFu;
	}

	std::vector<std::pair<uint32_t, uint32_t>> LayerRegistry::GetCollisionDefaultMasks() {
		RegistryState& state = GetState();
		std::lock_guard<std::mutex> lock(state.Mutex);
		std::vector<std::pair<uint32_t, uint32_t>> result;
		result.reserve(state.CollisionDefaultMaskByBit.size());
		for (const auto& [bit, mask] : state.CollisionDefaultMaskByBit)
			result.emplace_back(bit, mask);
		std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
		return result;
	}

} // namespace Ehu
