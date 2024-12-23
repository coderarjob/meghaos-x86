/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross platform kernel - System calls
 * ---------------------------------------------------------------------------
 */
#include <stdbool.h>
#include <utils.h>
#include <types.h>
#include <kdebug.h>
#include <process.h>
#include <x86/process.h>
#include <x86/vgatext.h>
#include <kernel.h>
#ifdef GRAPHICS_MODE_ENABLED
    #include <compositor.h>
#endif // GRAPHICS_MODE_ENABLED
#include <handle.h>
#include <panic.h>
#include <cm/osif.h>
#if ARCH == x86
    #include <x86/paging.h>
    #include <x86/boot.h>
#endif

typedef struct SystemcallFrame {
    U32 ebp;
    U32 eip;
    U32 cs;
    U32 eflags;
    U32 esp;
    U32 ss;
} __attribute__ ((packed)) SystemcallFrame;

#if defined(DEBUG)
void ksys_console_writeln (SystemcallFrame frame, char* fmt, char* text);
#if !defined(GRAPHICS_MODE_ENABLED)
void ksys_console_setcolor (SystemcallFrame frame, U8 bg, U8 fg);
void ksys_console_setposition (SystemcallFrame frame, U8 row, U8 col);
#endif // !GRAPHICS_MODE_ENABLED
#endif // DEBUG
INT ksys_createProcess (SystemcallFrame frame, void* processStartAddress, SIZE binLengthBytes,
                       KProcessFlags flags);
void ksys_yieldProcess (SystemcallFrame frame, U32 ebx, U32 ecx, U32 edx, U32 esi, U32 edi);
void ksys_killProcess (SystemcallFrame frame, UINT exitCode);
void ksys_abortProcess (SystemcallFrame frame, UINT exitCode);
bool ksys_processPopEvent (SystemcallFrame frame, OSIF_ProcessEvent* const e);
U32 ksys_process_getPID (SystemcallFrame frame);
U32 ksys_get_tickcount (SystemcallFrame frame);
PTR ksys_process_getDataMemoryStart (SystemcallFrame frame);
U32 sys_get_os_error (SystemcallFrame frame);

#ifdef GRAPHICS_MODE_ENABLED
Handle ksys_window_createWindow (SystemcallFrame frame, const char* winTitle);
bool ksys_window_destoryWindow (SystemcallFrame frame, Handle h);
bool ksys_getWindowFB (SystemcallFrame frame, Handle h, OSIF_WindowFrameBufferInfo * const wfb);
void ksys_window_graphics_flush_all (SystemcallFrame frame);
#endif // GRAPHICS_MODE_ENABLED

#if ARCH == x86
void ksys_find_bootloaded_file (SystemcallFrame frame, const char* const filename,
                                OSIF_BootLoadedFiles* const file);
#endif // ARCH = x86

static U32 s_getSysCallCount();
static INT s_handleInvalidSystemCall();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wpedantic"
void* g_syscall_table[] = {
    //---------------------------
#if defined(DEBUG)
    &ksys_console_writeln,            // 0
#else
    &s_handleInvalidSystemCall,       // 0
#endif
    //---------------------------
    &ksys_createProcess,              // 1
    &ksys_yieldProcess,               // 2
    &ksys_killProcess,                // 3
    //---------------------------
#if defined(DEBUG) && !defined(GRAPHICS_MODE_ENABLED)
    &ksys_console_setcolor,           // 4
    &ksys_console_setposition,        // 5
#else
    &s_handleInvalidSystemCall,       // 4
    &s_handleInvalidSystemCall,       // 5
#endif
    //---------------------------
    &ksys_processPopEvent,            // 6
    &ksys_process_getPID,             // 7
    &ksys_get_tickcount,              // 8
    &ksys_process_getDataMemoryStart, // 9
    //---------------------------
#ifdef GRAPHICS_MODE_ENABLED
    &ksys_window_createWindow,        // 10
    &ksys_window_destoryWindow,       // 11
    &ksys_getWindowFB,                // 12
    &ksys_window_graphics_flush_all,  // 13
#else
    &s_handleInvalidSystemCall,      // 10
    &s_handleInvalidSystemCall,      // 11
    &s_handleInvalidSystemCall,      // 12
    &s_handleInvalidSystemCall,      // 13
#endif
    //---------------------------
    &sys_get_os_error,               // 14
    //---------------------------
#if ARCH == x86
    &ksys_find_bootloaded_file,      // 15
#else
    &s_handleInvalidSystemCall,      // 15
#endif
    //---------------------------
    &ksys_abortProcess,              // 16
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
    RETURN_ERROR (ERR_INVALID_SYSCALL, KERNEL_EXIT_FAILURE);
}
#pragma GCC diagnostic pop

#if defined(DEBUG)
void ksys_console_writeln (SystemcallFrame frame, char* fmt, char* text)
{
    FUNC_ENTRY ("Frame return address: %x:%px, fmt: %px text: %px", frame.cs, frame.eip, fmt, text);
    (void)frame;
    kearly_printf (fmt, text);
}

#if !defined(GRAPHICS_MODE_ENABLED)
void ksys_console_setcolor (SystemcallFrame frame, U8 bg, U8 fg)
{
    FUNC_ENTRY ("Frame return address: %x:%px, fg: %x bg: %x", frame.cs, frame.eip, fg, bg);
    (void)frame;
    kdisp_ioctl (DISP_SETATTR, k_dispAttr (bg, fg, 0));
}

void ksys_console_setposition (SystemcallFrame frame, U8 row, U8 col)
{
    FUNC_ENTRY ("Frame return address: %x:%px, row: %x col: %x", frame.cs, frame.eip, row, col);
    (void)frame;
    kdisp_ioctl (DISP_SETCOORDS, row, col);
}
#endif // !GRAPHICS_MODE_ENABLED
#endif // DEBUG

INT ksys_createProcess (SystemcallFrame frame, void* processStartAddress, SIZE binLengthBytes,
                       KProcessFlags flags)
{
    FUNC_ENTRY ("Frame return address: %x:%x, flags: %x, start address: %px, binary len: %x",
                frame.cs, frame.eip, flags, processStartAddress, binLengthBytes);

    (void)frame;
    return kprocess_create (processStartAddress, binLengthBytes, flags);
}

void ksys_yieldProcess (SystemcallFrame frame, U32 ebx, U32 ecx, U32 edx, U32 esi, U32 edi)
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

void ksys_killProcess (SystemcallFrame frame, UINT exitCode)
{
    FUNC_ENTRY ("Frame return address: %x:%x, exit code: %x", frame.cs, frame.eip, exitCode);
    (void)frame;
    kprocess_exit ((U8)exitCode, false);
}

void ksys_abortProcess (SystemcallFrame frame, UINT exitCode)
{
    FUNC_ENTRY ("Frame return address: %x:%x, exit code: %x", frame.cs, frame.eip, exitCode);
    (void)frame;
    kprocess_exit ((U8)exitCode, true);
}

U32 ksys_process_getPID (SystemcallFrame frame)
{
    FUNC_ENTRY ("Frame return address: %x:%x", frame.cs, frame.eip);
    (void)frame;
    return kprocess_getCurrentPID();
}

PTR ksys_process_getDataMemoryStart (SystemcallFrame frame)
{
    FUNC_ENTRY ("Frame return address: %x:%x", frame.cs, frame.eip);
    (void)frame;
    KProcessSections* section = kprocess_getCurrentProcessDataSection();
    return section == NULL ? (PTR)0 : section->virtualMemoryStart;
}

bool ksys_processPopEvent (SystemcallFrame frame, OSIF_ProcessEvent* const e)
{
    FUNC_ENTRY ("Frame return address: %x:%x, e: %px ", frame.cs, frame.eip, e);
    (void)frame;

    KProcessEvent ke;
    UINT pid = kprocess_getCurrentPID();
    if (!kprocess_popEvent (pid, &ke)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Copy to user space
    e->event = (OSIF_ProcessEvents)ke.event;
    e->data  = ke.data;
    return true;
}

U32 ksys_get_tickcount (SystemcallFrame frame)
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
Handle ksys_window_createWindow (SystemcallFrame frame, const char* winTitle)
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

bool ksys_window_destoryWindow (SystemcallFrame frame, Handle h)
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

void ksys_window_graphics_flush_all (SystemcallFrame frame)
{
    FUNC_ENTRY ("Frame return address: %x:%x", frame.cs, frame.eip);
    (void)frame;
    kcompose_flush();
}
#endif // GRAPHICS_MODE_ENABLED

#if ARCH == x86
void ksys_find_bootloaded_file (SystemcallFrame frame, const char* const filename,
                                OSIF_BootLoadedFiles* const file)
{
    FUNC_ENTRY ("Frame return address: %x:%x", frame.cs, frame.eip);
    (void)frame;

    BootFileItem fileinfo = kboot_findBootFileItem (filename);
    file->length          = fileinfo.length;
    Physical start        = PHYSICAL (fileinfo.startLocation);
    file->startLocation   = HIGHER_HALF_KERNEL_TO_VA (start);
}
#endif // ARCH = x86
