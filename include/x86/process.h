/*
 * --------------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Process management
 * --------------------------------------------------------------------------------
 */
#pragma once

#include <types.h>
#include <buildcheck.h>

// Process EAX, ECX and EDX are not preserved by the Scheduler as these are treated as scratch
// registers. If requried these registers must be preserved by the caller just before doing a system
// call.
struct ProcessRegisterState {
    U32 ebx;
    U32 esi;
    U32 edi;
    U32 esp;
    U32 ebp;
    U32 eip;
    U32 eflags;
    U32 cs;
    U32 ds; // Not just DS. SS, ES, FS, GS are also set to this.
};
