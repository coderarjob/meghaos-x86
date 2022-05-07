/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Potable Types
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/

#ifndef __PORTABLE_TYPES__
#define __PORTABLE_TYPES__

#if defined(__i386__) || (defined(UNITTEST) && ARCH == x86)
    #include <x86/types.h>
#endif

#endif // __PORTABLE_TYPES__
