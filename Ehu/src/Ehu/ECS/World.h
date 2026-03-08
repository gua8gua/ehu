#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Entity.h"
#include "TypeId.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include <type_traits>

namespace Ehu {

	class IComponentPool {
	public:
		virtual ~IComponentPool() = default;
		virtual void Remove(Entity e) = 0;
		virtual bool Has(Entity e) const = 0;
	};

	template<typename T>
	class ComponentPool : public IComponentPool {
	public:
		void Remove(Entity e) override { m_Data.erase(e); }
		bool Has(Entity e) const override { return m_Data.find(e) != m_Data.end(); }
		std::unordered_map<Entity, T> m_Data;
	};

	/// ECS World：实体创建/销毁、组件增删查、按组件组合迭代
	class EHU_API World {
	public:
		World() = default;
		~World() = default;

		World(const World&) = delete;
		World& operator=(const World&) = delete;

		Entity CreateEntity();
		void DestroyEntity(Entity e);
		bool IsValid(Entity e) const;

		template<typename T>
		void AddComponent(Entity e, T&& component);

		template<typename T>
		void RemoveComponent(Entity e);

		template<typename T>
		T* GetComponent(Entity e);

		template<typename T>
		const T* GetComponent(Entity e) const;

		template<typename T>
		bool HasComponent(Entity e) const;

		/// 迭代拥有 C0 的实体
		template<typename C0, typename Func>
		void Each(Func&& func);
		template<typename C0, typename Func>
		void Each(Func&& func) const;
		/// 迭代同时拥有 C0、C1 的实体
		template<typename C0, typename C1, typename Func>
		void Each(Func&& func);
		template<typename C0, typename C1, typename Func>
		void Each(Func&& func) const;
		/// 迭代同时拥有 C0、C1、C2 的实体
		template<typename C0, typename C1, typename C2, typename Func>
		void Each(Func&& func);
		template<typename C0, typename C1, typename C2, typename Func>
		void Each(Func&& func) const;

		/// 返回拥有 IdComponent 的所有实体（用于 GetEntities 兼容）
		std::vector<Entity> GetEntities() const;

	private:
		template<typename T>
		ComponentPool<T>* GetOrCreatePool();

		template<typename T>
		ComponentPool<T>* GetPool();
		template<typename T>
		const ComponentPool<T>* GetPool() const;

		std::vector<uint32_t> m_Generations;
		std::vector<uint32_t> m_FreeList;
		uint32_t m_NextId = 1;
		std::vector<Entity> m_LiveEntities;

		std::unordered_map<ComponentTypeId, std::unique_ptr<IComponentPool>> m_Pools;
	};

	// --- Template implementations ---

	template<typename T>
	void World::AddComponent(Entity e, T&& component) {
		if (!IsValid(e)) return;
		using U = std::remove_cv_t<std::remove_reference_t<T>>;
		GetOrCreatePool<U>()->m_Data[e] = std::forward<T>(component);
	}

	template<typename T>
	void World::RemoveComponent(Entity e) {
		auto* pool = GetPool<T>();
		if (pool) pool->Remove(e);
	}

	template<typename T>
	T* World::GetComponent(Entity e) {
		if (!IsValid(e)) return nullptr;
		auto* pool = GetPool<T>();
		if (!pool) return nullptr;
		auto it = pool->m_Data.find(e);
		return it != pool->m_Data.end() ? &it->second : nullptr;
	}

	template<typename T>
	const T* World::GetComponent(Entity e) const {
		if (!IsValid(e)) return nullptr;
		auto* pool = GetPool<T>();
		if (!pool) return nullptr;
		auto it = pool->m_Data.find(e);
		return it != pool->m_Data.end() ? &it->second : nullptr;
	}

	template<typename T>
	bool World::HasComponent(Entity e) const {
		if (!IsValid(e)) return false;
		auto* pool = GetPool<T>();
		return pool && pool->Has(e);
	}

	template<typename T>
	ComponentPool<T>* World::GetOrCreatePool() {
		ComponentTypeId id = GetTypeId<T>();
		auto it = m_Pools.find(id);
		if (it != m_Pools.end())
			return static_cast<ComponentPool<T>*>(it->second.get());
		auto pool = std::make_unique<ComponentPool<T>>();
		ComponentPool<T>* ptr = pool.get();
		m_Pools[id] = std::move(pool);
		return ptr;
	}

	template<typename T>
	const ComponentPool<T>* World::GetPool() const {
		ComponentTypeId id = GetTypeId<T>();
		auto it = m_Pools.find(id);
		if (it == m_Pools.end()) return nullptr;
		return static_cast<ComponentPool<T>*>(it->second.get());
	}

	template<typename T>
	ComponentPool<T>* World::GetPool() {
		ComponentTypeId id = GetTypeId<T>();
		auto it = m_Pools.find(id);
		if (it == m_Pools.end()) return nullptr;
		return static_cast<ComponentPool<T>*>(it->second.get());
	}

	// Each with single component
	template<typename C0, typename Func>
	void World::Each(Func&& func) {
		auto* pool = GetOrCreatePool<C0>();
		for (auto& kv : pool->m_Data) {
			Entity e = kv.first;
			if (!IsValid(e)) continue;
			func(e, kv.second);
		}
	}

	template<typename C0, typename Func>
	void World::Each(Func&& func) const {
		const auto* pool = GetPool<C0>();
		if (!pool) return;
		for (const auto& kv : pool->m_Data) {
			Entity e = kv.first;
			if (!IsValid(e)) continue;
			func(e, const_cast<C0&>(kv.second));
		}
	}

	// Each with two components - common case for Transform + Camera, Transform + Sprite, etc.
	template<typename C0, typename C1, typename Func>
	void World::Each(Func&& func) {
		auto* pool0 = GetOrCreatePool<C0>();
		for (auto& kv : pool0->m_Data) {
			Entity e = kv.first;
			if (!IsValid(e)) continue;
			C1* c1 = GetComponent<C1>(e);
			if (!c1) continue;
			func(e, kv.second, *c1);
		}
	}

	template<typename C0, typename C1, typename Func>
	void World::Each(Func&& func) const {
		const auto* pool0 = GetPool<C0>();
		if (!pool0) return;
		for (const auto& kv : pool0->m_Data) {
			Entity e = kv.first;
			if (!IsValid(e)) continue;
			const C1* c1 = GetComponent<C1>(e);
			if (!c1) continue;
			func(e, const_cast<C0&>(kv.second), const_cast<C1&>(*c1));
		}
	}

	// Each with three components
	template<typename C0, typename C1, typename C2, typename Func>
	void World::Each(Func&& func) {
		auto* pool0 = GetOrCreatePool<C0>();
		for (auto& kv : pool0->m_Data) {
			Entity e = kv.first;
			if (!IsValid(e)) continue;
			C1* c1 = GetComponent<C1>(e);
			C2* c2 = GetComponent<C2>(e);
			if (!c1 || !c2) continue;
			func(e, kv.second, *c1, *c2);
		}
	}

	template<typename C0, typename C1, typename C2, typename Func>
	void World::Each(Func&& func) const {
		const auto* pool0 = GetPool<C0>();
		if (!pool0) return;
		for (const auto& kv : pool0->m_Data) {
			Entity e = kv.first;
			if (!IsValid(e)) continue;
			const C1* c1 = GetComponent<C1>(e);
			const C2* c2 = GetComponent<C2>(e);
			if (!c1 || !c2) continue;
			func(e, const_cast<C0&>(kv.second), const_cast<C1&>(*c1), const_cast<C2&>(*c2));
		}
	}

} // namespace Ehu
