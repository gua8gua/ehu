#include "ehupch.h"
#include "UUID.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace Ehu {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_Uniform;

	UUID::UUID() : m_UUID(s_Uniform(s_Engine)) {}

	UUID::UUID(uint64_t raw) : m_UUID(raw) {}

	std::string UUID::ToString() const {
		std::ostringstream oss;
		oss << std::hex << std::setfill('0') << std::setw(16) << m_UUID;
		return oss.str();
	}

} // namespace Ehu
