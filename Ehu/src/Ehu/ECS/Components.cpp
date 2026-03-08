#include "ehupch.h"
#include "Components.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

namespace Ehu {

	void TransformComponent::SetRotation(const glm::vec3& eulerRadians) {
		Rotation = glm::quat_cast(glm::eulerAngleXYZ(eulerRadians.x, eulerRadians.y, eulerRadians.z));
		Dirty = true;
	}

} // namespace Ehu
