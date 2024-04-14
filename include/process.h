/*
 * --------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Process management
 * --------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>

typedef enum ProcessStates {
    PROCESS_NOT_CREATED,
    PROCESS_NOT_STARTED,
    PROCESS_STARTED,
    PROCESS_ENDED
} ProcessStates;

typedef enum ProcessFlags {
    PROCESS_FLAGS_NONE           = 0,
    PROCESS_FLAGS_KERNEL_PROCESS = (1 << 1),
    PROCESS_FLAGS_THREAD         = (1 << 2),
} ProcessFlags;

__asm__(".equ PROCESS_FLAGS_NONE, 0;"
        ".equ PROCESS_FLAGS_KERNEL_PROCESS, (1 << 1);"
        ".equ PROCESS_FLAGS_THREAD, (1 << 2);");

typedef struct ProcessRegisterState ProcessRegisterState;

typedef struct ProcessInfo {
    UINT processID;
    Physical pagedir;
    ProcessStates state;
    Physical binaryAddress;
    Physical stackAddress;
    ProcessFlags flags;
    ProcessRegisterState* registerStates;
} ProcessInfo;

INT kprocess_create (void* processStartAddress, SIZE binLengthBytes, ProcessFlags flags);
bool kprocess_yield (ProcessRegisterState* currentState);
