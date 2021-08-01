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

    #define INTEL_32_IDT_LOCATION   0xC0000000 /* 256 entries, 8 bytes each */
    #define INTEL_32_GDT_LOCATION   0xC0000800 /* 512 entries, 8 bytes each */
    #define INTEL_32_KSTACK_TOP     0xC0027FFF /* Stack grows down from here */
    #define BOOT_INFO_LOCATION      0xC0001800
    #define VIDEO_RAM_LOCATION      0xC00B8000 /* VGA Text mode buffer */

#endif //__MEM_LOC_H__
