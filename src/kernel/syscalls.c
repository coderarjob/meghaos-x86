/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross platform kernel - System calls
 * ---------------------------------------------------------------------------
 */
#include <types.h>
#include <kdebug.h>

U32 sys_debug_info (char* fmt, U32* value);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wpedantic"
void* g_syscall_table[] = {
    &sys_debug_info,
};
#pragma GCC diagnostic pop

U32 sys_debug_info (char* fmt, U32* value)
{
    INFO (fmt, *value);
    return 0xB01D;
}
