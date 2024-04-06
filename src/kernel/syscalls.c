/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross platform kernel - System calls
 * ---------------------------------------------------------------------------
 */
#include <types.h>
#include <kdebug.h>
#include <x86/interrupt.h>

U32 sys_debug_info (InterruptFrame *frame, char* fmt, U32* value);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wpedantic"
void* g_syscall_table[] = {
    &sys_debug_info,
};
#pragma GCC diagnostic pop

U32 sys_debug_info (InterruptFrame* frame, char* fmt, U32* value)
{
    FUNC_ENTRY ("Frame return address: 0x%x:0x%x, fmt: 0x%px, value: 0x%x", frame->cs, frame->ip,
                fmt, value);

    INFO (fmt, *value);
    return 0xB01D;
}
