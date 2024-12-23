/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Platform Kernel - Kernel Management
 * ---------------------------------------------------------------------------
 */
#pragma once

#include <types.h>
#include <kassert.h>
#include <vmm.h>
#include <config.h>
#ifdef GRAPHICS_MODE_ENABLED
    #include <kgraphics.h>
#endif // GRAPHICS_MODE_ENABLED

typedef struct KernelStateInfo {
    enum {
        KERNEL_PHASE_STATE_BOOT_COMPLETE = 0,
#if defined(DEBUG) && !defined(GRAPHICS_MODE_ENABLED)
        KERNEL_PHASE_STATE_TEXTDISP_READY,
#endif
        KERNEL_PHASE_STATE_PMM_READY,
        KERNEL_PHASE_STATE_SALLOC_READY,
        KERNEL_PHASE_STATE_VMM_READY,
        KERNEL_PHASE_STATE_KMALLOC_READY,
#ifdef GRAPHICS_MODE_ENABLED
        KERNEL_PHASE_STATE_GRAPHICS_READY,
#endif
        KERNEL_PHASE_STATE_KERNEL_READY
    } phase;          // Phase in which the Kernel is in currently.
    UINT errorNumber; // Code for last error.
    VMemoryManager* context;
    U32 tick_count; // incremented every CONFIG_TICK_PERIOD_MICROSEC micro seconds.
#ifdef GRAPHICS_MODE_ENABLED
    // Kernel Graphics back buffer
    KGraphicsArea gx_backfb;
    // HW Video Framebuffer
    KGraphicsArea gx_hwfb;
#endif
} KernelStateInfo;

extern volatile KernelStateInfo g_kstate;

typedef enum KernelEvents {
    KERNEL_EVENT_NONE                  = 0,
    KERNEL_EVENT_PROCCESS_YIELD_REQ    = 1,
    KERNEL_EVENT_PROCCESS_CHILD_KILLED = 2
} KernelEvents;

#define KERNEL_PHASE_SET(p)                                                                    \
    do {                                                                                       \
        k_assert (p == 0 || (g_kstate.phase + 1) == p, "Current state is unsuitable for " #p); \
        g_kstate.phase = p;                                                                    \
    } while (0)

#define KERNEL_PHASE_CHECK(p)               (g_kstate.phase >= (p))

#define KERNEL_PHASE_VALIDATE(p)            k_assert (g_kstate.phase >= p, "Current state is not " #p);

#define KERNEL_MICRODEC_TO_TICK_COUNT(us)   ((U32)((us) / CONFIG_TICK_PERIOD_MICROSEC))
#define KERNEL_TICK_COUNT_TO_MICROSEC(tick) ((U64)(tick) * (U64)CONFIG_TICK_PERIOD_MICROSEC)

void k_delay (UINT ms);
void keventmanager_invoke();
