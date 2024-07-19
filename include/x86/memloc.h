/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Static Memory Locations
*
* Note:
* See Physical/Virtual Memory regions documentation for complete memory map
; ---------------------------------------------------------------------------
*/

#if !defined(MEM_LOC_H_X86)
    #define MEM_LOC_H_X86

    #if !defined(UNITTEST)
        #include <buildcheck.h>
        #include <config.h>
        #include <moslimits.h>
        #include <utils.h>

        #define MEM_START_KERNEL_LOW_REGION     (3 * GB)

        #define MEM_START_IDT                   MEM_START_KERNEL_LOW_REGION
        #define MEM_LEN_BYTES_IDT               (4 * KB)

        #define MEM_START_GDT                   0xC0001000
        #define MEM_LEN_BYTES_GDT               (4 * KB)

        #define MEM_START_BOOT_INFO             0xC0002000
        #define MEM_LEN_BYTES_BOOT_INFO         (4 * KB)

        #define MEM_START_KSTACK                0xC0003000
        #define MEM_LEN_BYTES_KSTACK            (128 * KB)
        /* Stack grows down from here */
        #define MEM_KSTACK_TOP                  ((MEM_START_KSTACK + MEM_LEN_BYTES_KSTACK) - 1)

        #define MEM_START_VGA_TEXT              0xC00B8000 /* VGA Text mode buffer */

        #define MEM_START_KERNEL_PAGE_DIR       0xC0023000
        #define MEM_LEN_BYTES_KERNEL_PAGE_DIR   (4 * KB)

        #define MEM_START_KERNEL_PAGE_TABLE     0xC0024000
        #define MEM_LEN_BYTES_KERNEL_PAGE_TABLE (4 * KB)

        #define MEM_START_KERNEL_PAB            0xC0025000
        #define MEM_LEN_BYTES_KERNEL_PAB        PAB_SIZE_BYTES

        #define MEM_START_SALLOC                0xC0026000
        #define MEM_LEN_BYTES_SALLOC            (128 * KB)

        #define MEM_END_KERNEL_LOW_REGION       (0xC0046000 - 1)
        #define MEM_LEN_BYTES_KERNEL_LOW_REGION \
            MEM_LEN_BYTES (MEM_START_KERNEL_LOW_REGION, MEM_END_KERNEL_LOW_REGION)

        #define MEM_START_KERNEL_HIGH_REGION   0xC0100000

        #define MEM_START_KMALLOC              0xC0300000
        #define MEM_LEN_BYTES_KMALLOC          (128 * KB)

        #define PROCESS_ADDR_SPACE_START       0x00010000
        #define PROCESS_TEXT_VA_START          0x00010000
        #define PROCESS_STACK_VA_START         0x00030000

        #define MEM_START_PAGING_EXT_TEMP_MAP  0xC03FE000U // Temporary map for external modules
        #define MEM_START_PAGING_INT_TEMP_MAP  0xC03FF000U // Temporary map for Paging module
        #define MEM_START_PAGING_RECURSIVE_MAP 0xFFC00000U // Recursive map of PDs

        #define MEM_END_KERNEL_HIGH_REGION     (4 * GB - 1)
        #define MEM_LEN_BYTES_KERNEL_HIGH_REGION \
            MEM_LEN_BYTES (MEM_START_KERNEL_HIGH_REGION, MEM_END_KERNEL_HIGH_REGION)
    #else
        #include <mosunittest.h>
        #define MEM_START_KERNEL_PAB MOCK_THIS_MACRO_USING (mem_start_kernel_pab)
    #endif // UNITTEST

#endif // MEM_LOC_H_X86
