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

static U32 s_getSysCallCount();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wpedantic"
void* g_syscall_table[] = {
    &sys_debug_log,
    &sys_console_writeln,
};
#pragma GCC diagnostic pop

/***************************************************************************************************
 * Global system call despatcher
 **************************************************************************************************/
__asm__(".text;"
        ".globl syscall_asm_despatcher;"
        "syscall_asm_despatcher:;"
        "    push ebp;"
        "    mov ebp, esp;"
        ////////////////////////////
        // But first check if the syscall number is within range.
        ////////////////////////////
        "    push eax;"
        "    push ebx;"
        "       mov ebx, eax;" // Store the syscall number in EBX
        "       call s_getSysCallCount;"
        "       cmp ebx, eax;"
        "    pop ebx;"
        "    pop eax;"
        "    jae .fini;"
        ////////////////////////////
        // Push the arguments to syscall function and call the corresponding syscall
        ////////////////////////////
        // The segment registers are not preserved. But this should be okay, as segmentation is not
        // supposed to be used.
        "    push edi;"
        "    push esi;"
        "    push edx;"
        "    push ecx;"
        "    push ebx;"
        // First argument is pointer to the InterruptFrame/
        "    lea ebx, [ebp + 4];"
        "    push ebx;"
        // Syscall function is retrived using syscall number in EAX
        "    lea eax, [4 * eax + g_syscall_table];"
        "    call [eax];"
        "    add esp, 4;"
        "    pop ebx;"
        "    pop ecx;"
        "    pop edx;"
        "    pop esi;"
        "    pop edi;"
        ".fini:;"
        "    pop ebp;"
        "    iret;");

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static U32 s_getSysCallCount()
{
    return ARRAY_LENGTH(g_syscall_table);
}
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