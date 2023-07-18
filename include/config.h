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

#endif // MOS_CONFIG_H
