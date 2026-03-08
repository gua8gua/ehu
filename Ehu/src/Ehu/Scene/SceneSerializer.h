#pragma once

#include "Core/Core.h"
#include <string>

namespace Ehu {

	class Scene;

	/// 场景序列化：保存/加载场景到文件（Id、Tag、Transform、Sprite）；路径与读写通过 FileSystem
	class EHU_API SceneSerializer {
	public:
		SceneSerializer() = default;

		/// 将场景保存到 path；仅序列化 IdComponent、TagComponent、TransformComponent、SpriteComponent
		bool Serialize(Scene* scene, const std::string& path);
		/// 从 path 加载到 scene（先清空当前实体，再创建并填充）；主相机会根据保存的 MainCamera UUID 设置
		bool Deserialize(Scene* scene, const std::string& path);
	};

} // namespace Ehu
