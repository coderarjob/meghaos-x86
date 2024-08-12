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
#include <kernel.h>
#include <paging.h>

/***************************************************************************************************
 * Copies n bytes from src to dest. Can handle overlaps.
 *
 * @Input dest     Pointer to the destination. Should not be NULL.
 * @Input src      Pointer to the source. Should not be NULL.
 * @Input n        Number of bytes to copy.
 * @return         Pointer to the start of the destination.
***************************************************************************************************/
void* k_memcpy (void* dest, const void* src, size_t n)
{
    FUNC_ENTRY ("Dest: %px, Src: %px, Len: %x bytes", (PTR)dest, (PTR)src, n);

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
 * Fills memory with constant byte
 *
 * @Input s     Pointer to the destination. Should not be NULL.
 * @Input c     Byte to fill the destination with.
 * @Input n     Number of bytes to fill.
 * @return      Pointer to the start of the destination.
***************************************************************************************************/
void *k_memset (void *s, U8 c, size_t n)
{
    FUNC_ENTRY ("Dest: %px, Byte: %px, Len: %x bytes", (PTR)s, c, n);

    return __builtin_memset(s, c, n);
}

/***************************************************************************************************
 * Copies n bytes from src to dest (raw physical memory).
 *
 * @Input dest     Pointer to the destination. Should not be NULL.
 * @Input src      Physical address of the source.
 * @Input n        Number of bytes to copy.
 * @return         Pointer to the start of the destination.
 ***************************************************************************************************/
void k_memcpyToPhyMem (Physical dest, PTR src, SIZE n)
{
    FUNC_ENTRY ("Dest: %px, Src: %px, Len: %x bytes", dest.val, (PTR)src, n);

    KERNEL_PHASE_VALIDATE (KERNEL_PHASE_STATE_VMM_READY);

    Physical l_dest = dest;
    PTR l_src       = src;
    SIZE remBytes   = n;

    // Copy whole pages worth of bytes
    while (remBytes >= CONFIG_PAGE_FRAME_SIZE_BYTES) {
        k_memcpy (kpg_temporaryMap (l_dest), (void*)l_src, CONFIG_PAGE_FRAME_SIZE_BYTES);
        kpg_temporaryUnmap();
        remBytes -= CONFIG_PAGE_FRAME_SIZE_BYTES;
        l_dest.val += CONFIG_PAGE_FRAME_SIZE_BYTES;
        l_src += CONFIG_PAGE_FRAME_SIZE_BYTES;
    }

    // Copy remaining bytes
    void* bin_va = kpg_temporaryMap (l_dest);
    k_memcpy (bin_va, (void*)l_src, remBytes);
    kpg_temporaryUnmap();
}

/***************************************************************************************************
 * Fills memory with multi-byte (maximum of 4 bytes) pattern
 * Insparation from BSD/MAC OS function `memset_pattern4`.
 *
 * @Input s     Pointer to the destination. Should not be NULL.
 * @Input p     U32 value containing the pattern to fill the destination with.
 * @Input szp   Number of bytes in the pattern to use
 * @Input n     Number of bytes to set in the destination. Must be multiple of 'szp'.
 * @return      Pointer to the start of the destination.
***************************************************************************************************/
void* k_memset_pat4 (void* s, U32 p, SIZE szp, SIZE n)
{
    FUNC_ENTRY ("Dest: %px, Pat: %x, PatLen: %x, Count: %x", (PTR)s, p, szp, n);

    k_assert(IS_ALIGNED(n, szp), "Desination end not aligned to pattern size");

    if (szp == 1) {
        return k_memset (s, (U8)p, n);
    }

    U8* pbytes = (U8*)&p;
    U8* dest   = (U8*)s;

    for (; n > 0; n -= szp, dest += szp) {
        switch (szp) {
        case 4:
            *(dest + 3) = pbytes[3];
            // fall through
        case 3:
            *(dest + 2) = pbytes[2];
            // fall through
        case 2:
            *(dest + 1) = pbytes[1];
            *(dest + 0) = pbytes[0];
            break;
        default:
            UNREACHABLE();
        }
    }
    return s;
}
