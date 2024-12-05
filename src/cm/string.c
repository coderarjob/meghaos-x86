/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - App Library - String/Memory functions
 * -------------------------------------------------------------------------------------------------
 */

#include <cm/types.h>

/***************************************************************************************************
 * Copies n bytes from src to dest. Can handle overlaps.
 *
 * @Input dest     Pointer to the destination. Should not be NULL.
 * @Input src      Pointer to the source. Should not be NULL.
 * @Input n        Number of bytes to copy.
 * @return         Pointer to the start of the destination.
 ***************************************************************************************************/
void* cm_memcpy (void* dest, const void* src, size_t n)
{
    U8* cdest = (U8*)dest;
    U8* csrc  = (U8*)src;

    // This handles the scenario where destination overlaps the source.
    cdest += n - 1;
    csrc += n - 1;

    while (n--)
        *cdest-- = *csrc--;

    return (CHAR*)dest;
}

/***************************************************************************************************
 * Gets lenght of a NULL terminated string.
 *
 * @Input s     Pointer to NULL terminated string
 * @return      Length of the string not including the NULL character.
 * TODO: Make the implementation mode efficient
 ***************************************************************************************************/
UINT cm_strlen (const char* s)
{
    UINT len = 0;
    for (; *s != '\0'; len++, s++)
        ;

    return len;
}

/***************************************************************************************************
 * Copies at most 'n' characters from one string to another.
 *
 * @Input d     Destination
 * @Input s     Source
 * @Input n     Number of characters (bytes) to copy from destination to source.
 * @return      Pointer to destination
 ***************************************************************************************************/
char* cm_strncpy (char* d, const char* s, SIZE n)
{
    char* dh = d;
    for (; n > 0 && (*d = *s) != '\0'; s++, d++, n--)
        ;

    return dh;
}

/***************************************************************************************************
 * Fills memory with constant byte
 *
 * @Input s     Pointer to the destination. Should not be NULL.
 * @Input c     Byte to fill the destination with.
 * @Input n     Number of bytes to fill.
 * @return      Pointer to the start of the destination.
 ***************************************************************************************************/
void* cm_memset (void* const s, U8 c, size_t n)
{
    return __builtin_memset (s, c, n);
}
