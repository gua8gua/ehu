#include "MemoryStats.h"

#ifdef EHU_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#include <psapi.h>
	#pragma comment(lib, "Psapi.lib")
#endif

namespace Ehu {

	uint64_t MemoryStats::GetProcessHeapBytes() {
#ifdef EHU_PLATFORM_WINDOWS
		PROCESS_MEMORY_COUNTERS pmc = {};
		pmc.cb = sizeof(pmc);
		if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
			return static_cast<uint64_t>(pmc.WorkingSetSize);
#endif
		return 0;
	}

} // namespace Ehu
