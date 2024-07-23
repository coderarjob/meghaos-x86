/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Static Memory Locations
 * ---------------------------------------------------------------------------
 */

#pragma once

#if defined(__i386__) && !defined(UNITTEST)
    #include <x86/memloc.h>

    #define ARCH_MEM_START_SALLOC      X86_MEM_START_SALLOC
    #define ARCH_MEM_LEN_BYTES_SALLOC  X86_MEM_LEN_BYTES_SALLOC

    #define ARCH_MEM_LEN_BYTES_KMALLOC X86_MEM_LEN_BYTES_KMALLOC
#endif
