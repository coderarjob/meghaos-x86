/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Configuration
* ---------------------------------------------------------------------------
*/

#ifndef MOS_CONFIG_H
#define MOS_CONFIG_H

#include <buildcheck.h>

#if defined(__i386__) || (defined(UNITTEST) && ARCH == x86)
    #include <x86/config.h>
#endif

#if defined(UNITTEST)
    #include <mosunittest.h>
    #undef CONFIG_HANDLES_ARRAY_ITEM_COUNT
    #define CONFIG_HANDLES_ARRAY_ITEM_COUNT MOCK_THIS_MACRO_USING (config_handles_array_item_count)
#endif

#endif // MOS_CONFIG_H
