/*
 * These are portable integer types for use in the Operating System. It makes
 * use of C99 stdint.h for the types.
 *
 * Note:
 * Remember that these header files are for building OS and its utilitites, it
 * is not a SDK.
 * */

#ifndef __PORTABLE_TYPES__
#define __PORTABLE_TYPES__

#ifdef __i386__
    #define u8   unsigned char
    #define u16  unsigned short
    #define u32  unsigned int
    #define u64  unsigned long
    #define u128 unsigned long long 
#endif

#endif // __PORTABLE_TYPES__
