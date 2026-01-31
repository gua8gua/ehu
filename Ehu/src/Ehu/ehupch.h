#pragma once
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stdint.h>
#ifdef EHU_PLATFORM_WINDOWS
	#include <Windows.h>
#endif 

#define EHU_BIND_EVENT_FN(fn) std::bind(&fn,this, std::placeholders::_1)