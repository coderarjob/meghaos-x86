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

    /* Casts a bit field of n bits to UINT. */
    inline UINT CAST_BITN_TO_U32 (UINT t, UINT n) {
        return (UINT)(t & (UINT)((1 << n) -1));
    }

    /* VIRTUAL_ADDRESS:
     * 32 bit Physical address. This type will prevent mixing physical and
     * virtual addresses as this type cannot be cast to any other pointer, not
     * even void * explicitly.

     * Use the 'PHYSICAL' macro for initializing variables or function
     * parameters of type PHYSICAL.

     * Example:
     * PHYSICAL vga = PHYSICAL (0xB8000);
     * Read it as, the 'vga' variable of type PHYSICAL holds the
     * physical address 0xB8000. So think of this type as a pointer
     * implementation.
     */
    typedef union
    {
        USYSINT val;
    } __attribute__ ((__transparent_union__, packed)) PHYSICAL;

    #define PHYSICAL(address) {.val = address}
#endif // __x86_TYPES__
