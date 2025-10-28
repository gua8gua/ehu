#pragma once

#ifdef EHU_PLATFORM_WINDOWS
    #ifdef EHU_BUILD_DLL
        #define EHU_API __declspec(dllexport)
    #else
        #define EHU_API __declspec(dllimport)
    #endif 
#else
    #error Platform not supported!
#endif

#ifdef EHU_ENABLE_ASSERTS
    #define EHU_ASSERT(x,...) { if(!(x)) { EHU_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
    #define EHU_CORE_ASSERT(x,...) { if(!(x)) { EHU_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
    #define EHU_ASSERT(x,...)
    #define EHU_CORE_ASSERT(x,...)
#endif


#define BIT(x) (1 << (x))