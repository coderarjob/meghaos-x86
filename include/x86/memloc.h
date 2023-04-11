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

#ifndef MEM_LOC_H_X86
#define MEM_LOC_H_X86

    #include <buildcheck.h>

    #define INTEL_32_IDT_LOCATION   0xC0001000 /* 256 entries, 8 bytes each */
    #define INTEL_32_GDT_LOCATION   0xC0001800 /* 512 entries, 8 bytes each */
    #define BOOT_INFO_LOCATION      0xC0002800 /* 1 KB in size */
    #define INTEL_32_KSTACK_TOP     0xC0043BFF /* Stack grows down from here */
    #define VIDEO_RAM_LOCATION      0xC00B8000 /* VGA Text mode buffer */

#endif //MEM_LOC_H_X86
