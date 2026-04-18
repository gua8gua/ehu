#pragma once

#include "Core/Core.h"
#include "Core/UUID.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace Ehu {

	class VertexArray;
	class Camera;

	using RenderChannelId = uint32_t;
	namespace BuiltinRenderChannels {
		static constexpr RenderChannelId Default = 0u;
		static constexpr RenderChannelId UI = 1u;
		static constexpr RenderChannelId Debug = 2u;
	}

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
		/// 相对 Assets 的纹理路径，空则仅绘制纯色四边形
		std::string TexturePath;
		float TilingFactor = 1.0f;
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
		bool Primary = true;
		bool FixedAspectRatio = false;
	};

	/// 2D 圆环渲染（着色器圆近似由 Renderer2D::DrawCircle 实现）
	struct EHU_API CircleRendererComponent {
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness = 0.05f;
		float Fade = 0.005f;
		float SortKey = 0.0f;
		bool Transparent = false;
	};

	/// 2D 文本（字体为相对 Assets 路径，运行时加载）
	struct EHU_API TextComponent {
		std::string TextString;
		std::string FontPath;
		float PixelHeight = 32.0f;
		float Kerning = 0.0f;
		float LineSpacing = 0.0f;
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float SortKey = 0.0f;
		bool Transparent = true;
	};

	/// 标签组件：仅承载编辑器显示名称
	struct EHU_API TagComponent {
		std::string Name;
	};

	/// 渲染筛选组件：与 Core::Layer 解耦，只表达可见性通道
	struct EHU_API RenderFilterComponent {
		RenderChannelId Channel = BuiltinRenderChannels::Default;
	};

	/// 物理筛选组件：表达碰撞分组和掩码
	struct EHU_API PhysicsFilterComponent {
		uint32_t CollisionLayer = 1u;
		uint32_t CollisionMask = 0xFFFFFFFFu;
	};

	enum class EHU_API Rigidbody2DBodyType : uint8_t {
		Static = 0,
		Dynamic,
		Kinematic
	};

	struct EHU_API Rigidbody2DComponent {
		Rigidbody2DBodyType Type = Rigidbody2DBodyType::Dynamic;
		glm::vec2 LinearVelocity = { 0.0f, 0.0f };
		float GravityScale = 1.0f;
		bool FixedRotation = true;
		/// 运行时 Box2D 物体（仅 Runtime/Simulation 有效；编辑/序列化为 nullptr）
		void* RuntimeBody = nullptr;
	};

	struct EHU_API BoxCollider2DComponent {
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f };
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;
		void* RuntimeFixture = nullptr;
	};

	struct EHU_API CircleCollider2DComponent {
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;
		void* RuntimeFixture = nullptr;
	};

	inline bool ShouldCollide(const PhysicsFilterComponent& a, const PhysicsFilterComponent& b) {
		return (a.CollisionMask & b.CollisionLayer) != 0u
			&& (b.CollisionMask & a.CollisionLayer) != 0u;
	}

	/// 实体唯一 ID（可选，CreateEntity 时可自动添加）
	struct EHU_API IdComponent {
		UUID Id;
	};

	enum class EHU_API ScriptFieldType {
		None = 0,
		Bool,
		Int,
		Float,
		Vec2,
		Vec3,
		Vec4,
		String,
		/// 引用场景中实体的 IdComponent UUID（Raw）
		Entity
	};

	struct EHU_API ScriptFieldValue {
		ScriptFieldType Type = ScriptFieldType::None;
		bool BoolValue = false;
		int32_t IntValue = 0;
		float FloatValue = 0.0f;
		glm::vec2 Vec2Value = glm::vec2(0.0f);
		glm::vec3 Vec3Value = glm::vec3(0.0f);
		glm::vec4 Vec4Value = glm::vec4(0.0f);
		std::string StringValue;
		uint64_t EntityUUIDValue = 0;
	};

	/// 脚本实例数据：支持同一实体挂载多个脚本（含同类型重复）
	struct EHU_API ScriptInstanceData {
		UUID InstanceId = UUID(0);
		std::string ClassName;
		std::unordered_map<std::string, ScriptFieldValue> Fields;
	};

	/// 脚本组件：记录脚本实例列表（顺序即生命周期执行顺序）
	struct EHU_API ScriptComponent {
		std::vector<ScriptInstanceData> Instances;
	};

} // namespace Ehu
