/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Utils header
*
* For macros or function declarations, which are too broad and cannot be put
* inside any specific header.
* ---------------------------------------------------------------------------
*/

#ifndef UTILS_H
#define UTILS_H

#include <buildcheck.h>
#include <types.h>
#include <kassert.h>
#include <config.h>

/** Used to know the offset of a member in a structure type */
#define offsetOf(type,member) ((size_t)(&((type *)0)->member))

/** Length of an array in bytes */
#define ARRAY_LENGTH(ar) (sizeof ((ar))/sizeof ((ar)[0]))

/** Checks if 'a' is aligned to 'n' */
#define IS_ALIGNED(a, n) (((a) & ((n) - 1)) == 0)

/* If 'a' is not already aligned to 'n', returns next aligned number */
#define ALIGN_UP(a, n) (IS_ALIGNED((a),(n)) ? (a)                   \
                                            : (((a) / (n)) + 1) * (n))

/** If 'a' is not already aligned to 'n', returns previous aligned number */
#define ALIGN_DOWN(a, n) (IS_ALIGNED((a),(n)) ? (a)                   \
                                              : ((a) / (n))  * (n))

/** Maximum of two integers. If equal, returns 'b' */
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

/** Minimum of two integers. If equal, returns 'b' */
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/** Bit mask
 * Example: BIT_MASK(19, 1) = 0xFFFFE */
#define BIT_MASK(l, r) (((1U << ((l) - (r) + 1U)) - 1U) << r)

/**Bit test
 * Checks is bit s (represented by the shifted value) is set in w.
 * NOTE: `& 0x1` makes the GCC happy when assigning to bit field.*/
#define BIT_ISSET(w, s)             \
    (__extension__({                \
         UINT w_ = (w);             \
         UINT s_ = (s);             \
         (w_ & s_) == s_ ? 1U : 0U; \
     }) & 0x1)

// Number of complete pages from at-most 'b' number of bytes.
#define BYTES_TO_PAGEFRAMES_FLOOR(b) \
    ((b) / CONFIG_PAGE_FRAME_SIZE_BYTES & BIT_MASK (CONFIG_PAGE_FRAME_SIZE_BITS, 0))

// Number of complete pages from at-least 'b' number of bytes.
#define BYTES_TO_PAGEFRAMES_CEILING(b) \
    BYTES_TO_PAGEFRAMES_FLOOR (ALIGN_UP ((b), CONFIG_PAGE_FRAME_SIZE_BYTES))

// Number of bytes in 'pf' number of pages.
#define PAGEFRAMES_TO_BYTES(pf) ((USYSINT)(pf) << CONFIG_PAGE_SIZE_BITS)

/**Bit test
 * Checks is bit s (represented by the shifted value) is not set in w. */
#define BIT_ISUNSET(w, s) ((BIT_ISSET (w, s) == 1U ? 0U : 1U) & 0x1)

/** Power of Two
 * Returns 2^n value */
#define POWER_OF_TWO(n) (1 << (n))

/** Stringfy macros */
#define STR_NDU(v) #v
#define STR(v) STR_NDU(v)

#define NORETURN() __builtin_unreachable()
#define UNREACHABLE() k_assert(false, "Unreachable code")

#endif // UTILS_H
