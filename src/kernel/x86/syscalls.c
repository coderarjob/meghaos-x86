/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross platform kernel - System calls
 * ---------------------------------------------------------------------------
 */
#include <disp.h>
#include <stdbool.h>
#include <utils.h>
#include <types.h>
#include <kdebug.h>
#include <process.h>
#include <x86/process.h>
#include <x86/vgatext.h>
#include <kernel.h>
#include <compositor.h>
#include <handle.h>
#include <panic.h>
#include <applib/osif.h>

typedef struct SystemcallFrame {
    U32 ebp;
    U32 eip;
    U32 cs;
    U32 eflags;
    U32 esp;
    U32 ss;
} __attribute__ ((packed)) SystemcallFrame;

void sys_console_writeln (SystemcallFrame frame, char* fmt, char* text);
INT sys_createProcess (SystemcallFrame frame, void* processStartAddress, SIZE binLengthBytes,
                       KProcessFlags flags);
void sys_yieldProcess (SystemcallFrame frame, U32 ebx, U32 ecx, U32 edx, U32 esi, U32 edi);
void sys_killProcess (SystemcallFrame frame);
void sys_console_setcolor (SystemcallFrame frame, U8 bg, U8 fg);
void sys_console_setposition (SystemcallFrame frame, U8 row, U8 col);
bool sys_processPopEvent (SystemcallFrame frame, U32 pid, PTR eventPtrOut);
U32 sys_process_getPID (SystemcallFrame frame);
U32 sys_get_tickcount (SystemcallFrame frame);
PTR sys_process_getDataMemoryStart (SystemcallFrame frame);

#ifdef GRAPHICS_MODE_ENABLED
Handle sys_window_createWindow (SystemcallFrame frame, const char* winTitle);
bool sys_window_destoryWindow (SystemcallFrame frame, Handle h);
bool ksys_getWindowFB (SystemcallFrame frame, Handle h, OSIF_WindowFrameBufferInfo * const wfb);
void sys_window_graphics_flush_all (SystemcallFrame frame);
#endif // GRAPHICS_MODE_ENABLED

static U32 s_getSysCallCount();
static INT s_handleInvalidSystemCall();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wpedantic"
void* g_syscall_table[] = {
    &sys_console_writeln,            // 0
    &sys_createProcess,              // 1
    &sys_yieldProcess,               // 2
    &sys_killProcess,                // 3
    &sys_console_setcolor,           // 4
    &sys_console_setposition,        // 5
    &sys_processPopEvent,            // 6
    &sys_process_getPID,             // 7
    &sys_get_tickcount,              // 8
    &sys_process_getDataMemoryStart, // 9
#ifdef GRAPHICS_MODE_ENABLED
    &sys_window_createWindow,        // 10
    &sys_window_destoryWindow,       // 11
    &ksys_getWindowFB,               // 12
    &sys_window_graphics_flush_all,  // 13
#else
    &s_handleInvalidSystemCall,      // 10
    &s_handleInvalidSystemCall,      // 11
    &s_handleInvalidSystemCall,      // 12
    &s_handleInvalidSystemCall,      // 13
#endif
};
#pragma GCC diagnostic pop

/***************************************************************************************************
 * Global system call despatcher
 **************************************************************************************************/
__asm__(
    // Structure describing the interrupt frame in the stack.
    ".struct 0;"
    "    field_ebp:    .struct . + 4;" // EBP is not part of interrupt frame but can be thought
                                       // of it because of the CDECL convention followed.
    "    field_eip:    .struct . + 4;" // --+
    "    field_cs:     .struct . + 4;" // --| Fixed interrupt frame.
    "    field_eflags: .struct . + 4;" // --+

    "    field_esp:    .struct . + 4;" // --+ For kernel processes, these are not added by the
    "    field_ss:     .struct . + 4;" // --+ CPU but are added by the despatcher routine.
    "interrupt_frame_struct_size: .struct .;"
    "    field_ebx:    .struct . + 4;" // --+
    "    field_ecx:    .struct . + 4;" // --|
    "    field_edx:    .struct . + 4;" // --| These are arguments to the system call function.
    "    field_esi:    .struct . + 4;" // --| EBX is the 1st arg, EDI is the 5th.
    "    field_edi:    .struct . + 4;" // --+
    "syscall_frame_struct_size: .struct .;"
    ///////////////////////////////////////
    ".text;"
    ".globl syscall_asm_despatcher;"
    "syscall_asm_despatcher:;"
    "    push ebp;"
    "    mov ebp, esp;"
    ////////////////////////////
    // Check if the syscall number is within range.
    ////////////////////////////
    "    push eax;"
    "    push ebx;"
    "       mov ebx, eax;" // Store the syscall number in EBX
    "       call s_getSysCallCount;"
    "       cmp ebx, eax;"
    "    pop ebx;"
    "    pop eax;"
    "    jae .fail;"
    ////////////////////////////
    /// Creates a space in the local stack frame for "System call frame". "System call frame" which
    /// includes the interrupt frame, SS:ESP & EBP of the caller is consistent irrespective of the
    /// callers privilage level.
    ////////////////////////////
    "   sub esp, syscall_frame_struct_size;"
    "   push edi;"
    "   push esi;"
    "   push ecx;"
    "       lea esi, [ebp];"
    "       lea edi, [ebp - syscall_frame_struct_size];"
    "       mov ecx, interrupt_frame_struct_size;"
    "       rep movsb;"
    "   pop ecx;"
    "   pop esi;"
    "   pop edi;"
    /// When the caller is Kernel process, SS:ESP is not part of the interrupt frame. Here we add
    /// these if caller is Kernel process.
    "   push eax;"
    "       mov eax, [ebp - syscall_frame_struct_size + field_cs];"
    "       test eax, 0x3;"
    "   pop eax;"
    "       jnz .cont1;"
    // Caller is Kernel process //
    "   push eax;"
    "       mov eax, ss;" // Process SS must be = Kernel SS
    "       mov [ebp - syscall_frame_struct_size + field_ss], eax;"
    "       lea eax, [ebp + 4 + 12];" // Due to CDECL, Caller ESP = EBP + 4 + (3 * 4)
    "       mov [ebp - syscall_frame_struct_size + field_esp], eax;"
    "   pop eax;"
    ////////////////////////////
    // Push the arguments to syscall function and call the corresponding syscall
    ////////////////////////////
    ".cont1:;"
    "   mov [ebp - syscall_frame_struct_size + field_edi], edi;"
    "   mov [ebp - syscall_frame_struct_size + field_esi], esi;"
    "   mov [ebp - syscall_frame_struct_size + field_edx], edx;"
    "   mov [ebp - syscall_frame_struct_size + field_ecx], ecx;"
    "   mov [ebp - syscall_frame_struct_size + field_ebx], ebx;"
    ////////////////////////////
    // Syscall function is retrived using syscall number in EAX
    "    lea eax, [4 * eax + g_syscall_table];"
    "    call [eax];"
    "    add esp, syscall_frame_struct_size;"
    "    jmp .fini;"
    ".fail:;"
    "   call s_handleInvalidSystemCall;"
    ".fini:;"
    "    pop ebp;"
    "    iret;");

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static U32 s_getSysCallCount()
{
    return ARRAY_LENGTH (g_syscall_table);
}
static INT s_handleInvalidSystemCall()
{
    RETURN_ERROR(ERR_INVALID_SYSCALL, KERNEL_EXIT_FAILURE);
}
#pragma GCC diagnostic pop

void sys_console_writeln (SystemcallFrame frame, char* fmt, char* text)
{
    FUNC_ENTRY ("Frame return address: %x:%px, fmt: %px text: %px", frame.cs, frame.eip, fmt, text);
    (void)frame;
    kearly_printf (fmt, text);
}

void sys_console_setcolor (SystemcallFrame frame, U8 bg, U8 fg)
{
    FUNC_ENTRY ("Frame return address: %x:%px, fg: %x bg: %x", frame.cs, frame.eip, fg, bg);
    (void)frame;
    kdisp_ioctl (DISP_SETATTR, k_dispAttr (bg, fg, 0));
}

void sys_console_setposition (SystemcallFrame frame, U8 row, U8 col)
{
    FUNC_ENTRY ("Frame return address: %x:%px, row: %x col: %x", frame.cs, frame.eip, row, col);
    (void)frame;
    kdisp_ioctl (DISP_SETCOORDS, row, col);
}

INT sys_createProcess (SystemcallFrame frame, void* processStartAddress, SIZE binLengthBytes,
                       KProcessFlags flags)
{
    FUNC_ENTRY ("Frame return address: %x:%x, flags: %x, start address: %px, binary len: %x",
                frame.cs, frame.eip, flags, processStartAddress, binLengthBytes);

    (void)frame;
    return kprocess_create (processStartAddress, binLengthBytes, flags);
}

void sys_yieldProcess (SystemcallFrame frame, U32 ebx, U32 ecx, U32 edx, U32 esi, U32 edi)
{
    FUNC_ENTRY ("Frame return address: %x:%x", frame.cs, frame.eip);
    (void)ecx;
    (void)edx;

    ProcessRegisterState state = {
        .ebx    = ebx,
        .esi    = esi,
        .edi    = edi,
        .esp    = frame.esp,
        .ebp    = frame.ebp,
        .eip    = frame.eip,
        .eflags = frame.eflags,
        .cs     = frame.cs,
        .ds     = frame.ss,
    };

    kprocess_yield (&state);
}

void sys_killProcess (SystemcallFrame frame)
{
    FUNC_ENTRY ("Frame return address: %x:%x", frame.cs, frame.eip);
    (void)frame;
    kprocess_exit();
}

U32 sys_process_getPID (SystemcallFrame frame)
{
    FUNC_ENTRY ("Frame return address: %x:%x", frame.cs, frame.eip);
    (void)frame;
    return kprocess_getCurrentPID();
}

PTR sys_process_getDataMemoryStart (SystemcallFrame frame)
{
    FUNC_ENTRY ("Frame return address: %x:%x", frame.cs, frame.eip);
    (void)frame;
    KProcessSections* section = kprocess_getCurrentProcessDataSection();
    return section == NULL ? (PTR)0 : section->virtualMemoryStart;
}

bool sys_processPopEvent (SystemcallFrame frame, U32 pid, PTR eventPtrOut)
{
    FUNC_ENTRY ("Frame return address: %x:%x, event ptr", frame.cs, frame.eip, eventPtrOut);
    (void)frame;
    return kprocess_popEvent (pid, (KProcessEvent*)eventPtrOut);
}

U32 sys_get_tickcount (SystemcallFrame frame)
{
    FUNC_ENTRY ("Frame return address: %x:%x", frame.cs, frame.eip);
    (void)frame;
    return g_kstate.tick_count;
}

U32 sys_get_os_error (SystemcallFrame frame)
{
    FUNC_ENTRY ("Frame return address: %x:%x", frame.cs, frame.eip);
    (void)frame;
    return g_kstate.errorNumber;
}

#ifdef GRAPHICS_MODE_ENABLED
Handle sys_window_createWindow (SystemcallFrame frame, const char* winTitle)
{
    FUNC_ENTRY ("Frame return address: %x:%x, title: %px", frame.cs, frame.eip, winTitle);
    (void)frame;
    Window* win = kcompose_createWindow (winTitle);
    if (!win) {
        RETURN_ERROR (ERROR_PASSTHROUGH, INVALID_HANDLE);
    }

    // Create handle for Window obj
    Handle newh;
    if ((newh = khandle_createHandle (win)) == KERNEL_EXIT_FAILURE) {
        kcompose_destroyWindow (win);
        RETURN_ERROR (ERROR_PASSTHROUGH, INVALID_HANDLE);
    }

    return newh;
}

bool sys_window_destoryWindow (SystemcallFrame frame, Handle h)
{
    FUNC_ENTRY ("Frame return address: %x:%x, Handle: %x", frame.cs, frame.eip, h);
    (void)frame;

    // Get window associated with this handle
    Window* win = NULL;
    if (!(win = khandle_getObject (h))) {
        RETURN_ERROR (ERROR_PASSTHROUGH, INVALID_HANDLE);
    }

    return kcompose_destroyWindow (win);
}

bool ksys_getWindowFB (SystemcallFrame frame, Handle h, OSIF_WindowFrameBufferInfo * const wfb)
{
    FUNC_ENTRY ("Frame return address: %x:%x, Handle: %x", frame.cs, frame.eip, h);
    (void)frame;
    Window* win = NULL;
    if (!(win = khandle_getObject (h))) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Copy to user space
    KGraphicsArea wa = win->workingArea;
    wfb->buffer = wa.buffer;
    wfb->bufferSizeBytes = wa.bufferSizeBytes;
    wfb->width_px = wa.width_px;
    wfb->height_px = wa.height_px;
    wfb->bytesPerPixel = wa.bytesPerPixel;
    wfb->bytesPerRow = wa.bytesPerRow;

    return true;
}

void sys_window_graphics_flush_all (SystemcallFrame frame)
{
    FUNC_ENTRY ("Frame return address: %x:%x", frame.cs, frame.eip);
    (void)frame;
    kcompose_flush();
}
#endif // GRAPHICS_MODE_ENABLED
