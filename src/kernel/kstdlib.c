/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross platform Kernel - Common stdlib functions
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/

#include <kstdlib.h>
#include <types.h>

/* Copies n bytes from src to dest. Returns dest */
void *k_memcpy (void *dest, const void *src, size_t n)
{
    U8 *cdest = (U8 *)dest;
    U8 *csrc = (U8 *)src;

    if (cdest > csrc && cdest < csrc + n) {
        // This handles the scenario where destination overlaps the source.
        cdest += n - 1;
        csrc += n - 1;
        while (n--)
            *cdest-- = *csrc--;
    }
    else {
        __builtin_memcpy(dest, src, n);
    }

    return (CHAR *)dest + n;
}

/* Fills memory with constant byte */
void *k_memset (void *s, INT c, size_t n)
{
    return __builtin_memset(s, c, n);
}

