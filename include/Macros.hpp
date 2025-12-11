#pragma once
#include <Geode/platform/cplatform.h>

#ifndef MORE_ICONS_EVENTS
    #ifdef GEODE_IS_WINDOWS
        #ifdef MORE_ICONS_EXPORTING
            #define MORE_ICONS_DLL __declspec(dllexport)
        #else
            #define MORE_ICONS_DLL __declspec(dllimport)
        #endif
    #else
        #ifdef MORE_ICONS_EXPORTING
            #define MORE_ICONS_DLL __attribute__((visibility("default")))
        #else
            #define MORE_ICONS_DLL
        #endif
    #endif
#else
    #define MORE_ICONS_DLL
#endif
