#pragma once

#include "ECS/Components.h"
#include <box2d/b2_body.h>
#include <cmath>
#include <glm/gtc/quaternion.hpp>

namespace Ehu {

	namespace Physics2D {

		inline b2BodyType ToBox2DBody(Rigidbody2DBodyType type) {
			switch (type) {
			case Rigidbody2DBodyType::Static: return b2_staticBody;
			case Rigidbody2DBodyType::Dynamic: return b2_dynamicBody;
			case Rigidbody2DBodyType::Kinematic: return b2_kinematicBody;
			}
			return b2_staticBody;
		}

		/// 从四元数提取 XY 平面旋转角（弧度），用于与 Box2D 同步
		inline float QuatToZAngle(const glm::quat& q) {
			return std::atan2(2.0f * (q.w * q.z + q.x * q.y), 1.0f - 2.0f * (q.y * q.y + q.z * q.z));
		}

	} // namespace Physics2D

} // namespace Ehu
