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

    if (cdest > csrc && cdest < csrc + n) {
        // This handles the scenario where destination overlaps the source.
        cdest += n-1;
        csrc += n-1;
        while(n--)
            *cdest-- = *csrc--;
    }
    else {
        while(n--)
            *cdest++ = *csrc++;
    }

    return (char *)dest + n;
}

/* Fills memory with constant byte */
void *memset(void *s, int c, size_t n)
{
    u8 *cs = (u8 *)s;
    u8 ch = (u8)c;

    while(n--)
        *cs++ = ch;

    return cs;
}

