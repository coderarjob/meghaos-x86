/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross platform kernel - System calls
 * ---------------------------------------------------------------------------
 */
#include <disp.h>
#include <utils.h>
#include <types.h>
#include <kdebug.h>
#include <process.h>
#include <x86/process.h>

typedef struct SystemcallFrame {
    U32 ebp;
    U32 eip;
    U32 cs;
    U32 eflags;
    U32 esp;
    U32 ss;
} __attribute__ ((packed)) SystemcallFrame;

void sys_console_writeln (SystemcallFrame frame, char* text);
INT sys_createProcess (SystemcallFrame frame, void* processStartAddress, SIZE binLengthBytes,
                       ProcessFlags flags);
void sys_yieldProcess (SystemcallFrame frame, U32 ebx, U32 ecx, U32 edx, U32 esi, U32 edi);

static U32 s_getSysCallCount();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#pragma GCC diagnostic ignored "-Wpedantic"
void* g_syscall_table[] = {
    &sys_console_writeln,
    &sys_createProcess,
    &sys_yieldProcess,
};
#pragma GCC diagnostic pop

/***************************************************************************************************
 * Global system call despatcher
 **************************************************************************************************/
__asm__(
    // Structure describing the interrupt frame in the stack.
    ".struct 0;"
    "    field_ebp:    .struct . + 4;"      // EBP is not part of interrupt frame but can be thought
                                            // of it because of the CDECL convention followed.
    "    field_eip:    .struct . + 4;"      // --+
    "    field_cs:     .struct . + 4;"      // --| Fixed interrupt frame.
    "    field_eflags: .struct . + 4;"      // --+ 

    "    field_esp:    .struct . + 4;"      // --+ For kernel processes, these are not added by the
    "    field_ss:     .struct . + 4;"      // --+ CPU but are added by the despatcher routine.
    "interrupt_frame_struct_size: .struct .;"
    "    field_ebx:    .struct . + 4;"      // --+
    "    field_ecx:    .struct . + 4;"      // --|
    "    field_edx:    .struct . + 4;"      // --| These are arguments to the system call function.
    "    field_esi:    .struct . + 4;"      // --| EBX is the 1st arg, EDI is the 5th.
    "    field_edi:    .struct . + 4;"      // --+
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
    "    jae .fini;"
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
    "       mov eax, ss;"               // Process SS must be = Kernel SS
    "       mov [ebp - syscall_frame_struct_size + field_ss], eax;"
    "       lea eax, [ebp + 4 + 12];"   // Due to CDECL, Caller ESP = EBP + 4 + (3 * 4)
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
    ".fini:;"
    "    pop ebp;"
    "    iret;");

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static U32 s_getSysCallCount()
{
    return ARRAY_LENGTH (g_syscall_table);
}
#pragma GCC diagnostic pop

void sys_console_writeln (SystemcallFrame frame, char* text)
{
    FUNC_ENTRY ("Frame return address: 0x%x:0x%x, text: 0x%x", frame.cs, frame.eip, text);
    (void)frame;
    kearly_println ("%s", text);
}

INT sys_createProcess (SystemcallFrame frame, void* processStartAddress, SIZE binLengthBytes,
                       ProcessFlags flags)
{
    FUNC_ENTRY (
        "Frame return address: 0x%x:0x%x, flags: 0x%x, start address: 0x%px, binary len: 0x%x",
        frame.cs, frame.eip, flags, processStartAddress, binLengthBytes);

    (void)frame;
    return kprocess_create (processStartAddress, binLengthBytes, flags);
}

void sys_yieldProcess (SystemcallFrame frame, U32 ebx, U32 ecx, U32 edx, U32 esi, U32 edi)
{
    FUNC_ENTRY ("Frame return address: 0x%x:0x%x", frame.cs, frame.eip);

    ProcessRegisterState state = {
        .ds  = frame.ss,
        .cs  = frame.cs,
        .eip = frame.eip,
        .esp = frame.esp,
        .ebx = ebx,
        .ecx = ecx,
        .edx = edx,
        .esi = esi,
        .edi = edi,
        .eax = 0, // EAX cannot be preserved.
    };

    kprocess_yield (&state);
}
