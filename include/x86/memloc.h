/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Static Memory Locations
*
* Note:
* See Physical/Virtual Memory regions documentation for complete memory map
; ---------------------------------------------------------------------------
*/

#if !defined(MEM_LOC_H_X86) && !defined (UNITTEST)
#define MEM_LOC_H_X86

    #include <buildcheck.h>

    #define INTEL_32_IDT_LOCATION   0xC0000000
    #define INTEL_32_GDT_LOCATION   0xC0001000
    #define BOOT_INFO_LOCATION      0xC0002000
    #define INTEL_32_KSTACK_TOP     0xC0042FFF /* Stack grows down from here */
    #define VIDEO_RAM_LOCATION      0xC00B8000 /* VGA Text mode buffer */
    #define KERNEL_PAGE_DIR         0xC0043000
    #define KERNEL_PAGE_TABLE       0xC0044000
    #define KERNEL_PAB              0xC0045000
    #define KERNEL_LOW_REGION_END   0xC0046000
    #define SALLOC_MEM_START        (3 * GB + 2 * MB)
    #define KMALLOC_MEM_START       (3 * GB + 3 * MB)

#endif //MEM_LOC_H_X86
