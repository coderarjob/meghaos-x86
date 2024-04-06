/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross platform kernel - System calls
 * ---------------------------------------------------------------------------
 */
#include <disp.h>
#include <utils.h>
#include <types.h>
#include <kdebug.h>
#include <x86/interrupt.h>

void sys_debug_log (InterruptFrame* frame, U32 type, char* text);
void sys_console_writeln (InterruptFrame* frame, char* text);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wpedantic"
void* g_syscall_table[] = {
    &sys_debug_log,
    &sys_console_writeln,
};
#pragma GCC diagnostic pop

void sys_console_writeln (InterruptFrame* frame, char* text)
{
    FUNC_ENTRY ("Frame return address: 0x%x:0x%x, text: 0x%x", frame->cs, frame->ip, text);
    kearly_println ("%s", text);
}

void sys_debug_log (InterruptFrame* frame, U32 type, char* text)
{
    FUNC_ENTRY ("Frame return address: 0x%x:0x%x, type: 0x%x, text: 0x%x", frame->cs, frame->ip,
                type, text);

    switch (type) {
    case 0:
        INFO ("%s", text);
        break;
    case 1:
        ERROR ("%s", text);
        break;
    default:
        UNREACHABLE();
    }
}
