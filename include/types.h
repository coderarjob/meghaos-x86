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

#ifdef __i386__
    #define u8   unsigned char
    #define u16  unsigned short
    #define u32  unsigned int
    #define u64  unsigned long
    #define u128 unsigned long long 
    
    #define size_t u32      // Type that can hold the largest number in an 
                            // architecture.
#endif

#endif // __PORTABLE_TYPES__
