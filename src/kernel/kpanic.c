/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Panic
 * ---------------------------------------------------------------------------
 */

#include <panic.h>
#include <types.h>
#include <moslimits.h>
#include <kdebug.h>
#include <disp.h>
#include <stdarg.h>
#include <kernel.h>
#ifdef GRAPHICS_MODE_ENABLED
    #include <graphics.h>
    #include <guicolours.h>
#endif

static void arch_dump_call_trace (PTR* raddrs, INT count);
#ifdef GRAPHICS_MODE_ENABLED
static void kpanic_gx_bsod (const char* buffer);
#endif

#ifdef GRAPHICS_MODE_ENABLED
static void kpanic_gx_bsod (const char* buffer)
{
    if (KERNEL_PHASE_CHECK (KERNEL_PHASE_STATE_GRAPHICS_READY)) {
        KGraphicsArea* ga = (KGraphicsArea*)&g_kstate.gx_hwfb;
        graphics_rect (ga, 0, 0, ga->width_px, ga->height_px, COLOR_BG_BSOD);
        kgraphics_drawstring (ga, 10, 10, buffer, COLOR_WHITE, COLOR_BG_BSOD);
    } else {
        // This has to be a failure very early on.
        // Not sure what can we do here!!
    }
}
#endif

void kpanic_ndu (UINT line, const CHAR* file, const CHAR* fmt, ...)
{
    CHAR buffer[MAX_PRINTABLE_STRING_LENGTH];
    va_list l;

    PTR raddrs[CONFIG_MAX_CALL_TRACE_DEPTH] = { 0 };
    arch_dump_call_trace (raddrs, ARRAY_LENGTH (raddrs));

    INT len = 0;

    len = kearly_snprintf (buffer, ARRAY_LENGTH (buffer), "\n\nKernel Panic:");
    len += kearly_snprintf (buffer + len, ARRAY_LENGTH (buffer), "\nAt: %s: %u\n\n", file, line);

    va_start (l, fmt);
    len += kearly_vsnprintf (buffer + len, ARRAY_LENGTH (buffer), fmt, l);
    va_end (l);

    len += kearly_snprintf (buffer + len, ARRAY_LENGTH (buffer), "\n\nCall stack:\n-");
    for (UINT i = 0; i < ARRAY_LENGTH (raddrs) && raddrs[i] > 0; i++) {
        len += kearly_snprintf (buffer + len, ARRAY_LENGTH (buffer), " %x", raddrs[i]);
    }
    len += kearly_snprintf (buffer + len, ARRAY_LENGTH (buffer), "\n------------\n");

#ifdef GRAPHICS_MODE_ENABLED
    kpanic_gx_bsod (buffer);
#endif

#ifdef DEBUG
    kdebug_printf ("%s", buffer);
    #ifndef GRAPHICS_MODE_ENABLED
    kearly_printf (buffer);
    #endif
#endif

    k_halt();
}

#if ARCH == x86
// Would be present for every CPU arch
static void arch_dump_call_trace (PTR* raddrs, INT count)
{
    typedef struct stack_frame {
        struct stack_frame* ebp;
        unsigned int eip;
    } stack_frame;

    stack_frame* frame;
    __asm__ volatile("mov %0, ebp;" : "=m"(frame));

    for (; count && frame->ebp; count--, frame = frame->ebp)
        *(raddrs++) = frame->eip;
}
#endif
