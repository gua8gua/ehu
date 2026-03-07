#pragma once

#include "Core/Core.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Ehu {

	class VertexArray;  // 前向声明，MeshComponent 持有 VAO（Platform 层）
	class Layer;        // 前向声明，可渲染实体所属的层（Extract 时仅该层提交本实体）

	/// 2D 精灵组件：尺寸、颜色等；位置取自实体 Transform 的平移
	struct EHU_API SpriteComponent {
		glm::vec2 Size = { 1.0f, 1.0f };
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float SortKey = 0.0f;
		bool Transparent = false;
	};

	/// 3D 网格组件：VAO、索引数、颜色等；变换取自实体 Transform
	struct EHU_API MeshComponent {
		VertexArray* VAO = nullptr;
		uint32_t IndexCount = 0;
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float SortKey = 0.0f;
		bool Transparent = false;
	};

	/// 场景实体：由所在场景管理；提供位置/缩放/旋转等操作接口，内部变换供渲染提交使用
	class EHU_API SceneEntity {
	public:
		SceneEntity() = default;

		// --- 变换操作接口（用户通过此类与组件操作实体）---
		void SetPosition(const glm::vec3& pos) { m_Position = pos; m_TransformDirty = true; }
		void SetPosition(float x, float y, float z) { SetPosition(glm::vec3(x, y, z)); }
		const glm::vec3& GetPosition() const { return m_Position; }

		void SetScale(const glm::vec3& scale) { m_Scale = scale; m_TransformDirty = true; }
		void SetScale(float s) { SetScale(glm::vec3(s, s, s)); }
		const glm::vec3& GetScale() const { return m_Scale; }

		/// 旋转：欧拉角（弧度），顺序 XYZ
		void SetRotation(const glm::vec3& eulerRadians);
		void SetRotation(const glm::quat& q) { m_Rotation = q; m_TransformDirty = true; }
		const glm::quat& GetRotation() const { return m_Rotation; }
		glm::vec3 GetRotationEuler() const { return glm::eulerAngles(m_Rotation); }

		/// 直接设置世界变换矩阵（高级用法）；会尽量反解出 position/scale/rotation
		void SetTransform(const glm::mat4& t);
		const glm::mat4& GetTransform() const;

		// --- 组件 ---
		bool HasSpriteComponent() const { return m_HasSprite; }
		bool HasMeshComponent() const { return m_HasMesh; }
		SpriteComponent& GetSpriteComponent() { return m_Sprite; }
		const SpriteComponent& GetSpriteComponent() const { return m_Sprite; }
		MeshComponent& GetMeshComponent() { return m_Mesh; }
		const MeshComponent& GetMeshComponent() const { return m_Mesh; }

		void SetSpriteComponent(const SpriteComponent& s) { m_Sprite = s; m_HasSprite = true; }
		void SetMeshComponent(const MeshComponent& m) { m_Mesh = m; m_HasMesh = true; }
		void RemoveSpriteComponent() { m_HasSprite = false; }
		void RemoveMeshComponent() { m_HasMesh = false; }

		/// 可渲染实体所属层：Extract 阶段仅 entity->GetRenderLayer() == 当前 Layer 时提交
		void SetRenderLayer(Layer* layer) { m_RenderLayer = layer; }
		Layer* GetRenderLayer() const { return m_RenderLayer; }

	private:
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_Scale = { 1.0f, 1.0f, 1.0f };
		glm::quat m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		mutable glm::mat4 m_Transform = glm::mat4(1.0f);
		mutable bool m_TransformDirty = true;

		bool m_HasSprite = false;
		bool m_HasMesh = false;
		SpriteComponent m_Sprite;
		MeshComponent m_Mesh;

		Layer* m_RenderLayer = nullptr;
	};

} // namespace Ehu
