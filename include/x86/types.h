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

#include <stdint.h>

    /* typedefs produce a more consistent result, for complicated types,
     * than #define.
     * Example:
     *   #define p_t CHAR*
     *   p_t a,b,c             // a is CHAR *, b,c are CHAR.
     *
     *  typedef char* p_t;
     *  p_t a,b,c              // a,b,c are CHAR* as it should be.
     */
    typedef uint8_t  U8;
    typedef uint16_t U16;

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

    /* Type that can hold the largest addressable memory.
     * For x86, that is 4GBytes, which an uint32_t type can hold.
     */
    typedef uint32_t size_t;

    /* Casts a bit field of n bits to 32 bit UINT. */
    inline U32 CAST_BITN_TO_U32 (U32 t, U32 n) {
        n--;
        return (U32)(t & ((2 << n) -1));
    }

    /* VIRTUAL_ADDRESS:
     * 32 bit Physical address. This type will prevent mixing physical and
     * virtual addresses as this type cannot be cast to any other pointer, not
     * even void * explicitly.

     * Use the 'PHYSICAL_ADDRESS' macro for initializing variables or function
     * parameters of type PHYSICAL_ADDRESS.

     * Example:
     * PHYSICAL_ADDRESS vga = PHYSICAL_ADDRESS (0xB8000);
     * Read it as, the 'vga' variable of type PHYSICAL_ADDRESS holds the
     * physical address 0xB8000. So think of this type as a pointer
     * implementation.
     */
    typedef
    struct { size_t phy_addr; } __attribute__ ((packed)) PHYSICAL_ADDRESS;

    #define PHYSICAL_ADDRESS(address) {.phy_addr = address}
#endif // __x86_TYPES__
