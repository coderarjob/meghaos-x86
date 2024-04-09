/*
 * --------------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Process management
 * --------------------------------------------------------------------------------
 */
#pragma once

#include <types.h>
#include <buildcheck.h>

void jump_to_process (U32 type, void* stackTop, void (*entry)(), U32 dataselector, U32 codeselector,
                      U32 cr3);

struct ProcessRegisterState {
    U32 eax;
    U32 ebx;
    U32 ecx;
    U32 edx;
    U32 esi;
    U32 edi;
    U32 esp;
    U32 ebp;
    U32 eip;
    U32 cs;
    U32 ds; // Not just DS. SS, ES, FS, GS are also set to this.
};
