/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Platform specific Types 
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/

#ifndef __x86_TYPES__
#define __x86_TYPES__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

    typedef uint8_t  U8;
    typedef uint16_t U16;

    /* For any environment, UINT and INT will match the native width of
     * registers. So when fast speed is required, or variabled are platform
     * independent, use UINT or INT instead of U32 or S32.*/
    typedef uint32_t U32;
    typedef U32      UINT;
    typedef U32      ULONG;

    typedef int32_t  S32;
    typedef S32      INT;
    typedef S32      LONG;

    typedef uint64_t U64;
    typedef U64      ULLONG;

    typedef int64_t  S64;
    typedef S64      LLONG;

    /* The C99 standard, defines the minimum sizes of integer types. So the exact size of compiler
     * dependent. CHAR type must be of type CHAR, which is different from int8_t or uint8_t.
     */
    typedef unsigned char UCHAR;
    typedef char          CHAR;

    /* Large enough to hold the largest address possible on a 32 bit system */
    typedef U32           USYSINT;

#endif // __x86_TYPES__
