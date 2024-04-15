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
    // Physical memorys for the process.
    struct {
        Physical PageDirectory;
        Physical Binary;
        Physical Stack;
        SIZE StackSizePages;
    } physical;
    struct {
        PTR Entry;
        PTR StackStart;
    } virt;
    // Initial states. These do not change throuout the lifetime of the process.
    UINT processID;
    ProcessFlags flags;
    // States which change
    ProcessStates state;
    ProcessRegisterState* registerStates;
} ProcessInfo;

INT kprocess_create (void* processStartAddress, SIZE binLengthBytes, ProcessFlags flags);
bool kprocess_yield (ProcessRegisterState* currentState);
