/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Static Memory Locations
* ---------------------------------------------------------------------------
* Dated: 19th October 2020
; ---------------------------------------------------------------------------
; Bulid 20201019
; - Initial version 
; ---------------------------------------------------------------------------
*/

#ifndef __MEM_LOC_H__
#define __MEM_LOC_H__

    #define INTEL_32_IDT_LOCATION   0x0000 /* 256 entries, 8 bytes each */
    #define INTEL_32_GDT_LOCATION   0x0800 /* 512 entries, 8 bytes each */
    #define INTEL_32_KSTACK_TOP     0x27FFF /* Stack grows down from here */

#endif //__MEM_LOC_H__
