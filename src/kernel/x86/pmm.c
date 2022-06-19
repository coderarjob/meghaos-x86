/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Physical Memory Management
*
* Methods to allocate and free physical pages.
* ---------------------------------------------------------------------------
*/

#include <kernel.h>

U8 *pab = NULL;

void
kpmm_init ()
{
    pab = (U8 *)CAST_PA_TO_VA (g_pab);
    k_memset (pab, 0x00, MAX_PAB_SIZE_BYTES);
}

PHYSICAL_ADDRESS
kpmm_alloc (INT size, PMMAllocationTypes type, PHYSICAL_ADDRESS start)
{

}
