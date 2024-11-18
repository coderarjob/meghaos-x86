/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Checks build environment.
* ---------------------------------------------------------------------------
*/

#if __STDC_VERSION__ >= 199901
    #define __C99__
#endif

#if defined(__linux__) && !defined(UNITTEST)
#error "You are targetting Linux. Which is wrong!"
#endif

#if !defined(__i386__) && !(defined(UNITTEST) && ARCH == x86)
#error "You are not targetting i386."
#endif

#if !defined (DEBUG) && !defined(GRAPHICS_MODE_ENABLED) && !defined (UNITTEST)
#error "Non-Graphics mode is only for debug."
#endif

#if !defined (DEBUG) && defined(PORT_E9_ENABLED) && !defined (UNITTEST)
#error "PORT 0xE9 is for debug mode only."
#endif
