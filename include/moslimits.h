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

#ifndef __MOS_LIMITS_H__
#define __MOS_LIMITS_H__

#if defined(__i386__) || (defined(UNITTEST) && ARCH == x86)
    #include <x86/moslimits.h>
#endif

#endif //__MOS_LIMITS_H__
