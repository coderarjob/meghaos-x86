/*
 * These are portable integer types for use in the Operating System. It makes
 * use of C99 stdint.h for the types.
 *
 * Note:
 * Remember that these header files are for building OS and its utilitites, it
 * is not a SDK.
 * */

#ifndef __PORTABLE_INTS__
#define __PORTABLE_INTS__

    #include<stdint.h>

    #define u8 uint8_t
    #define u16 uint16_t
    #define u32 uint32_t
#endif // __PORTABLE_INTS__
