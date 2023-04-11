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

#ifndef PORTABLE_TYPES
#define PORTABLE_TYPES

#if defined(__i386__) || (defined(UNITTEST) && ARCH == x86)
    #include <x86/types.h>
#endif

/* Casts a bit field of n bits to UINT. */
inline UINT CAST_BITN_TO_U32 (UINT t, UINT n) {
    return (UINT)(t & (UINT)((1 << n) -1));
}

/** Specific type to store Physical addresses.
 * This type will prevent mixing physical and virtual addresses and may be also lessen the need for
 * Hungarian notations.
 *
 * For ease of use, any variable of type USYSINT can be passed to a function expecting Physical
 * without any case (because of transparent union).

 * Use the 'Physical' macro for initializing variables or function parameters of type Physical.
 */
typedef union
{
    USYSINT val;
} __attribute__ ((packed)) Physical;

#define PHYSICAL(address) {.val = address}
#define PHYSICAL_NULL createPhysical(0)

static inline Physical createPhysical (USYSINT address) {
    Physical p = {.val = address};
    return p;
}

static inline bool isPhysicalNull (Physical address) {
    return address.val == 0;
}

#endif // PORTABLE_TYPES
