/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Limits
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 7th September 2021
*/

#ifndef MOS_LIMITS_H
#define MOS_LIMITS_H

#include <buildcheck.h>

#if defined(__i386__) || (defined(UNITTEST) && ARCH == x86)
    #include <x86/moslimits.h>
#endif

#endif //MOS_LIMITS_H
