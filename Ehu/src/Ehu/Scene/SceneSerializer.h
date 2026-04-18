#pragma once

#include "Core/Core.h"
#include <string>

namespace Ehu {

	class Scene;

	/// 场景序列化：保存/加载场景到文件。当前仅持久化 ECS 数据（Id、Tag、Transform、Sprite、Camera）；
	/// Scene 的 SceneNode 场景图不入档也不恢复，路径与读写通过 FileSystem。
	class EHU_API SceneSerializer {
	public:
		SceneSerializer() = default;

		/// 将场景保存到 path；序列化 Id、Tag、Transform、Sprite、Camera（Ortho/Persp 参数）及主相机 UUID
		bool Serialize(Scene* scene, const std::string& path);
		bool SerializeRuntime(Scene* scene, const std::string& path) { return Serialize(scene, path); }
		/// 从 path 加载到 scene（先清空当前实体与场景持有相机，再创建并填充）；主相机会根据保存的 MainCamera UUID 设置
		bool Deserialize(Scene* scene, const std::string& path);
		bool DeserializeRuntime(Scene* scene, const std::string& path) { return Deserialize(scene, path); }
	};

} // namespace Ehu
