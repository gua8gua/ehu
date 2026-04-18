#pragma once

#include "ehupch.h"
#include "Core/Core.h"
#include "Entity.h"
#include "TypeId.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include <type_traits>
#include <tuple>
#include <array>

namespace Ehu {

	class IComponentPool {
	public:
		virtual ~IComponentPool() = default;
		virtual void Remove(Entity e) = 0;
		virtual bool Has(Entity e) const = 0;
		virtual size_t Size() const = 0;
		virtual const std::vector<Entity>& Entities() const = 0;
	};

	// 组件池实现，管理T类型的组件
	template<typename T>
	class ComponentPool : public IComponentPool {
	public:
		// 尾插组件
		void Insert(Entity e, const T& component) {
			auto it = m_Indices.find(e);
			if (it != m_Indices.end()) {
				m_Components[it->second] = component;
				return;
			}
			m_Indices[e] = m_Entities.size();
			m_Entities.push_back(e);
			m_Components.push_back(component);
		}

		void Insert(Entity e, T&& component) {
			auto it = m_Indices.find(e);
			if (it != m_Indices.end()) {
				m_Components[it->second] = std::move(component);
				return;
			}
			m_Indices[e] = m_Entities.size();
			m_Entities.push_back(e);
			m_Components.push_back(std::move(component));
		}

		T* Get(Entity e) {
			auto it = m_Indices.find(e);
			return it != m_Indices.end() ? &m_Components[it->second] : nullptr;
		}

		const T* Get(Entity e) const {
			auto it = m_Indices.find(e);
			return it != m_Indices.end() ? &m_Components[it->second] : nullptr;
		}

		void Remove(Entity e) override {
			auto it = m_Indices.find(e);
			if (it == m_Indices.end())
				return;
			const size_t removeIndex = it->second;
			const size_t lastIndex = m_Entities.size() - 1;
			// 如果删除的不是最后一个组件，则将最后一个组件移动到删除的位置	
			if (removeIndex != lastIndex) {
				Entity movedEntity = m_Entities[lastIndex];
				m_Entities[removeIndex] = movedEntity;
				m_Components[removeIndex] = std::move(m_Components[lastIndex]);
				m_Indices[movedEntity] = removeIndex;
			}
			m_Entities.pop_back();
			m_Components.pop_back();
			m_Indices.erase(it);
		}
		bool Has(Entity e) const override { return m_Indices.find(e) != m_Indices.end(); }
		size_t Size() const override { return m_Entities.size(); }
		const std::vector<Entity>& Entities() const override { return m_Entities; }

	private:
		// 实体对应的组件索引
		std::unordered_map<Entity, size_t> m_Indices;
		// 实体数组
		std::vector<Entity> m_Entities;
		// 组件数组
		std::vector<T> m_Components;
	};

	/// ECS World：实体创建/销毁、组件增删查、按组件组合迭代
	class EHU_API World {
	public:
		World() = default;
		~World() = default;

		World(const World&) = delete;
		World& operator=(const World&) = delete;

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


		/// 注意：Components... 必须是“非 cv 限定”的组件类型（例如 TransformComponent）。
		template<typename... Components, typename Func>
		void Each(Func&& func);
		template<typename... Components, typename Func>
		void Each(Func&& func) const;

		/// 返回拥有 IdComponent 的所有实体（用于 GetEntities 兼容）
		std::vector<Entity> GetEntities() const;

	private:
		friend class Scene;
		Entity CreateEntity();

		template<typename T>
		ComponentPool<T>* GetOrCreatePool();

		template<typename T>
		ComponentPool<T>* GetPool();
		template<typename T>
		const ComponentPool<T>* GetPool() const;

		// 实体生成代数
		std::vector<uint32_t> m_Generations;
		// 空闲实体列表
		std::vector<uint32_t> m_FreeList;
		// 下一个实体ID
		uint32_t m_NextId = 1;
		// 存活实体列表
		std::vector<Entity> m_LiveEntities;

		// 各类型组件池
		std::unordered_map<ComponentTypeId, std::unique_ptr<IComponentPool>> m_Pools;
	};

	// --- Template implementations ---

	template<typename T>
	void World::AddComponent(Entity e, T&& component) {
		if (!IsValid(e)) return;
		using U = std::remove_cv_t<std::remove_reference_t<T>>;
		GetOrCreatePool<U>()->Insert(e, std::forward<T>(component));
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
		return pool->Get(e);
	}

	template<typename T>
	const T* World::GetComponent(Entity e) const {
		if (!IsValid(e)) return nullptr;
		auto* pool = GetPool<T>();
		if (!pool) return nullptr;
		return pool->Get(e);
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

	template<typename... Components, typename Func>
	void World::Each(Func&& func) {
		static_assert(sizeof...(Components) >= 1, "Each requires at least 1 component type");
		static_assert((!std::is_const_v<Components> && ...), "Each component types must be non-const");
		std::array<const IComponentPool*, sizeof...(Components)> pools{ GetPool<Components>()... };
		for (const IComponentPool* pool : pools) {
			if (!pool)
				return;
		}

		const IComponentPool* pivotPool = pools[0];
		for (const IComponentPool* pool : pools) {
			if (pool->Size() < pivotPool->Size())
				pivotPool = pool;
		}

		for (Entity e : pivotPool->Entities()) {
			if (!IsValid(e)) continue;

			auto comps = std::tuple<Components*...>{ GetComponent<Components>(e)... };
			bool allPresent = std::apply([](auto*... ptrs) { return (... && (ptrs != nullptr)); }, comps);
			if (!allPresent) continue;

			std::apply([&](auto*... ptrs) { func(e, (*ptrs)...); }, comps);
		}
	}

	template<typename... Components, typename Func>
	void World::Each(Func&& func) const {
		static_assert(sizeof...(Components) >= 1, "Each requires at least 1 component type");
		static_assert((!std::is_const_v<Components> && ...), "Each component types must be non-const");
		std::array<const IComponentPool*, sizeof...(Components)> pools{ GetPool<Components>()... };
		for (const IComponentPool* pool : pools) {
			if (!pool)
				return;
		}

		const IComponentPool* pivotPool = pools[0];
		for (const IComponentPool* pool : pools) {
			if (pool->Size() < pivotPool->Size())
				pivotPool = pool;
		}

		for (Entity e : pivotPool->Entities()) {
			if (!IsValid(e)) continue;

			auto comps = std::tuple<const Components*...>{ GetComponent<Components>(e)... };
			bool allPresent = std::apply([](auto*... ptrs) { return (... && (ptrs != nullptr)); }, comps);
			if (!allPresent) continue;

			std::apply([&](auto*... ptrs) { func(e, (*ptrs)...); }, comps);
		}
	}

} // namespace Ehu
