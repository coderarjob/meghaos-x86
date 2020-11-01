/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Common Kernel - Memory handling functions
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/

#include <kernel.h>

/* Copies n bytes from src to dest. Returns dest */
void *memcpy(void *dest, const void *src, size_t n)
{
    u8 *cdest = (u8 *)dest;
    u8 *csrc = (u8 *)src;

    // We are going to copy from the back. Right to Left.
    // This handles the scenario where destination overlaps the source.
    // i.e: dest > src AND dest < src + n
    cdest = cdest+n-1;
    csrc = csrc+n-1;

    while(dest != src && n-- > 0)
        *cdest-- = *csrc--;

    return cdest+n;
}

/* Fills memory with constant byte */
void *memset(void *s, int c, size_t n)
{
    u8 *cs = (u8 *)s;
    u8 ch = (u8)c;

    while(n--)
        *cs = ch;

    return s;
}

