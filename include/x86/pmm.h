/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Physical Memory Management Header
* ---------------------------------------------------------------------------
*/

#ifndef PMM_H_X86
#define PMM_H_X86

#include <kernel.h>

extern PHYSICAL g_pab; /* Address of Page Allocation Bitmap array */

typedef enum PMMAllocationTypes
{
    PMM_AUTOMATIC,
    PMM_FIXED
} PMMAllocationTypes;

void kpmm_init ();
bool kpmm_alloc (PHYSICAL *allocated, USYSINT byteCount, PMMAllocationTypes type,PHYSICAL start);
#endif // PMM_H_X86
