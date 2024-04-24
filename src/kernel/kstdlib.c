/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross platform Kernel - Common stdlib functions
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/

#include <kstdlib.h>
#include <types.h>
#include <kdebug.h>

/***************************************************************************************************
 * Copies n bytes from src to dest. Can handle overlaps.
 *
 * @Input dest     Pointer to the destination. Should not be NULL.
 * @Input src      Pointer to the source. Should not be NULL.
 * @Input n        Number of bytes to copy.
 * @return         Pointer to the start of the destination.
***************************************************************************************************/
void *k_memcpy (void *dest, const void *src, size_t n)
{
    FUNC_ENTRY ("Dest: %px, Src: %px, Len: %x bytes", (PTR)dest, (PTR)src, n);

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

    return (CHAR *)dest;
}

/***************************************************************************************************
 * Fills memory with constant byte
 *
 * @Input s     Pointer to the destination. Should not be NULL.
 * @Input c     Byte to fill the destination with.
 * @Input n     Number of bytes to fill.
 * @return      Pointer to the start of the destination.
***************************************************************************************************/
void *k_memset (void *s, INT c, size_t n)
{
    FUNC_ENTRY ("Dest: %px, Byte: %px, Len: %x bytes", (PTR)s, c, n);

    return __builtin_memset(s, c, n);
}

