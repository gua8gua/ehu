#include "ehupch.h"
#include "World.h"
#include <algorithm>

namespace Ehu {

	Entity World::CreateEntity() {
		Entity e;
		if (!m_FreeList.empty()) {
			e.id = m_FreeList.back();
			m_FreeList.pop_back();
			e.generation = m_Generations[e.id];
		} else {
			e.id = m_NextId++;
			if (e.id >= m_Generations.size())
				m_Generations.resize(e.id + 1);
			m_Generations[e.id] = 0;
			e.generation = 0;
		}
		m_LiveEntities.push_back(e);
		return e;
	}

	void World::DestroyEntity(Entity e) {
		if (!IsValid(e)) return;
		for (auto& kv : m_Pools)
			kv.second->Remove(e);
		m_Generations[e.id]++;
		m_FreeList.push_back(e.id);
		auto it = std::find(m_LiveEntities.begin(), m_LiveEntities.end(), e);
		if (it != m_LiveEntities.end()) {
			*it = m_LiveEntities.back();
			m_LiveEntities.pop_back();
		}
	}

	bool World::IsValid(Entity e) const {
		if (e.id == 0) return false;
		if (e.id >= m_Generations.size()) return false;
		return m_Generations[e.id] == e.generation;
	}

	std::vector<Entity> World::GetEntities() const {
		return m_LiveEntities;
	}

} // namespace Ehu
