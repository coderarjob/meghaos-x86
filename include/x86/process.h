/*
 * --------------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Process management
 * --------------------------------------------------------------------------------
 */
#pragma once

#include <types.h>
#include <buildcheck.h>

void jump_to_usermode (U32 dataselector, U32 codeselector, void* stackTop, void (*user_func)());

struct ProcessRegisterState {
    U32 eax;
    U32 ebx;
    U32 ecx;
    U32 edx;
    U32 esi;
    U32 edi;
    U32 esp;
    U32 ebp;
    U32 ss;
    U32 ip;
    U32 cs;
};
