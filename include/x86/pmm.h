/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Physical Memory Management Header
* ---------------------------------------------------------------------------
*/

#ifndef PMM_H_X86
#define PMM_H_X86

#include <kernel.h>

extern PHYSICAL_ADDRESS g_pab; /* Address of Page Allocation Bitmap array */

typedef enum PMMAllocationTypes
{
    AUTOMATIC,
    FIXED
} PMMAllocationTypes;

void kpmm_init ();
PHYSICAL_ADDRESS kpmm_alloc (INT size,
                             PMMAllocationTypes type,
                             PHYSICAL_ADDRESS start);
#endif // PMM_H_X86
