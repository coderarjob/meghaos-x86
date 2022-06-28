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

#define TRUE  1
#define FALSE 0

/* Casts a bit field of n bits to UINT. */
inline UINT CAST_BITN_TO_U32 (UINT t, UINT n) {
    return (UINT)(t & (UINT)((1 << n) -1));
}

/** Specific type to store Physical addresses.
 * This type will prevent mixing physical and virtual addresses and may be also lessen the need for
 * Hungarian notations.
 *
 * For ease of use, any variable of type USYSINT can be passed to a function expecting PHYSICAL
 * without any case (because of transparent union).

 * Use the 'PHYSICAL' macro for initializing variables or function parameters of type PHYSICAL.
 */
typedef union
{
    USYSINT val;
} __attribute__ ((__transparent_union__, packed)) PHYSICAL;

#define PHYSICAL(address) {.val = address}

#endif // __PORTABLE_TYPES__
