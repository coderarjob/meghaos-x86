/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Static Memory Locations
 * ---------------------------------------------------------------------------
 */

#pragma once

#include <buildcheck.h>

#if defined(UNITTEST)
    #include <mosunittest.h>
    #define ARCH_MEM_START_KERNEL_PAB  MOCK_THIS_MACRO_USING (arch_mem_start_kernel_pab)
    #define ARCH_MEM_START_SALLOC      MOCK_THIS_MACRO_USING (arch_mem_start_salloc)
    #define ARCH_MEM_LEN_BYTES_SALLOC  MOCK_THIS_MACRO_USING (arch_mem_len_bytes_salloc)
    #define ARCH_MEM_LEN_BYTES_KMALLOC MOCK_THIS_MACRO_USING (arch_mem_len_bytes_kmalloc)
#else
    #if defined(__i386__) || (defined(UNITTEST) && ARCH == x86)
        #include <x86/memloc.h>

        #define ARCH_MEM_START_SALLOC         X86_MEM_START_SALLOC
        #define ARCH_MEM_LEN_BYTES_SALLOC     X86_MEM_LEN_BYTES_SALLOC

        #define ARCH_MEM_LEN_BYTES_KMALLOC    X86_MEM_LEN_BYTES_KMALLOC

        #define ARCH_MEM_START_KERNEL_PAB     X86_MEM_START_KERNEL_PAB
        #define ARCH_MEM_LEN_BYTES_KERNEL_PAB X86_MEM_LEN_BYTES_KERNEL_PAB
    #endif
#endif
