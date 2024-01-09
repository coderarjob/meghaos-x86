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

/** Power of two */
UINT power_of_two(UINT e);

/** Bit mask
 * Example: BIT_MASK(19, 1) = 0xFFFFE */
#define BIT_MASK(r, l) (((1 << ((r) - (l) + 1)) - 1) << l)

/** Stringfy macros */
#define STR_NDU(v) #v
#define STR(v) STR_NDU(v)

#endif // UTILS_H
