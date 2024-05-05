/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Platform Kernel - Kernel Management
 * ---------------------------------------------------------------------------
 */
#pragma once

#include <types.h>

typedef struct KernelStateInfo {
    enum {
        KERNEL_READY_STATE_UNKOWN = 0,
        KERNEL_READY_STATE_TEXTDISP,
        KERNEL_READY_STATE_PMM,
        KERNEL_READY_STATE_SALLOC,
        KERNEL_READY_STATE_KMALLOC,
        KERNEL_READY_STATE_KERNEL_INITIALIZED
    } phase;                      // Phase in which the Kernel is in currently.
    UINT errorNumber;             // Code for last error.
    Physical kernelPageDirectory; // Physical location of Kernel Page Directory.
} KernelStateInfo;

extern KernelStateInfo g_kstate;
