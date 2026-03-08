#pragma once

#include "Core/Core.h"
#include "Core/UUID.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

namespace Ehu {

	class VertexArray;
	class Camera;
	class Layer;

	/// 变换组件：位置、旋转、缩放；世界矩阵按需重算
	struct EHU_API TransformComponent {
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };
		glm::quat Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

		mutable glm::mat4 WorldMatrix = glm::mat4(1.0f);
		mutable bool Dirty = true;

		void SetPosition(const glm::vec3& pos) { Position = pos; Dirty = true; }
		void SetPosition(float x, float y, float z) { SetPosition(glm::vec3(x, y, z)); }
		void SetScale(const glm::vec3& scale) { Scale = scale; Dirty = true; }
		void SetScale(float s) { SetScale(glm::vec3(s, s, s)); }
		void SetRotation(const glm::quat& q) { Rotation = q; Dirty = true; }
		/// 欧拉角（弧度），顺序 XYZ
		void SetRotation(const glm::vec3& eulerRadians);

		const glm::mat4& GetWorldMatrix() const {
			if (Dirty) {
				WorldMatrix = glm::translate(glm::mat4(1.0f), Position)
					* glm::mat4_cast(Rotation)
					* glm::scale(glm::mat4(1.0f), Scale);
				Dirty = false;
			}
			return WorldMatrix;
		}
	};

	/// 2D 精灵组件
	struct EHU_API SpriteComponent {
		glm::vec2 Size = { 1.0f, 1.0f };
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float SortKey = 0.0f;
		bool Transparent = false;
	};

	/// 3D 网格组件（VAO 由 Platform 层提供）
	struct EHU_API MeshComponent {
		VertexArray* VAO = nullptr;
		uint32_t IndexCount = 0;
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float SortKey = 0.0f;
		bool Transparent = false;
	};

	/// 相机组件：持有 Camera*（非拥有，由创建方管理生命周期），由 CameraSyncSystem 从 Transform 同步
	struct EHU_API CameraComponent {
		Camera* Camera = nullptr;
	};

	/// 标签：名称与渲染层（Extract 时按 RenderLayer 过滤）
	struct EHU_API TagComponent {
		std::string Name;
		Layer* RenderLayer = nullptr;
	};

	/// 实体唯一 ID（可选，CreateEntity 时可自动添加）
	struct EHU_API IdComponent {
		UUID Id;
	};

} // namespace Ehu
