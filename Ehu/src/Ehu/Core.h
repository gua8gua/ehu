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
