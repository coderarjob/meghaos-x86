/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Platform Kernel - Kernel Management
 * ---------------------------------------------------------------------------
 */
#pragma once

#include <types.h>
#include <kassert.h>
#include <vmm.h>

typedef struct KernelStateInfo {
    enum {
        KERNEL_PHASE_STATE_BOOT_COMPLETE = 0,
        KERNEL_PHASE_STATE_TEXTDISP_READY,
        KERNEL_PHASE_STATE_PMM_READY,
        KERNEL_PHASE_STATE_SALLOC_READY,
        KERNEL_PHASE_STATE_VMM_READY,
        KERNEL_PHASE_STATE_KMALLOC_READY,
        KERNEL_PHASE_STATE_KERNEL_READY
    } phase;                // Phase in which the Kernel is in currently.
    UINT errorNumber;       // Code for last error.
    VMemoryManager *context;
} KernelStateInfo;

extern KernelStateInfo g_kstate;

#define KERNEL_PHASE_SET(p)                                                                    \
    do {                                                                                       \
        k_assert (p == 0 || (g_kstate.phase + 1) == p, "Current state is unsuitable for " #p); \
        g_kstate.phase = p;                                                                    \
    } while (0)

#define KERNEL_PHASE_CHECK(p) (g_kstate.phase >= (p))

#define KERNEL_PHASE_VALIDATE(p) k_assert (g_kstate.phase >= p, "Current state is not " #p);
