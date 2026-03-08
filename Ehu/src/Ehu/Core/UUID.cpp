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

	UUID UUID::FromString(const std::string& str) {
		if (str.size() != 16) return UUID(0);
		uint64_t v = 0;
		for (size_t i = 0; i < 16; ++i) {
			char c = str[i];
			uint64_t d = 0;
			if (c >= '0' && c <= '9') d = c - '0';
			else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
			else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
			else return UUID(0);
			v = (v << 4) | d;
		}
		return UUID(v);
	}

} // namespace Ehu
