/*
 * --------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Process management
 * --------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#include <intrusive_queue.h>
#include <vmm.h>

#define PROCESS_ID_KERNEL  0x0
#define PROCESS_ID_INVALID -1

typedef enum ProcessStates {
    PROCESS_STATE_INVALID = 0,
    PROCESS_STATE_RUNNING = 1,
    PROCESS_STATE_IDLE    = 2,
} ProcessStates;

typedef enum ProcessFlags {
    PROCESS_FLAGS_NONE           = 0,
    PROCESS_FLAGS_KERNEL_PROCESS = (1 << 0),
    PROCESS_FLAGS_THREAD         = (1 << 1),
} ProcessFlags;

__asm__(".equ PROCESS_FLAGS_KERNEL_PROCESS, (1 << 0);"
        ".equ PROCESS_FLAGS_THREAD,         (1 << 1);");

typedef struct ProcessRegisterState ProcessRegisterState;

typedef struct ProcessSections {
    PTR virtualMemoryStart;
    SIZE sizePages;
} ProcessSections;

typedef struct ProcessInfo {
    // ----------------------
    // Initial states. These do not change throuout the lifetime of the process.
    // ----------------------
    ProcessSections binary;
    ProcessSections stack;
    ProcessSections data;
    VMemoryManager* context;
    // ProcessInfos' are part of the scheduler process table through this node.
    ListNode schedulerQueueNode;
    UINT processID;
    ProcessFlags flags;
    // ----------------------
    // States which change
    // ----------------------
    ProcessStates state;
    ProcessRegisterState* registerStates;
} ProcessInfo;

void kprocess_init();
INT kprocess_create (void* processStartAddress, SIZE binLengthBytes, ProcessFlags flags);
bool kprocess_yield (ProcessRegisterState* currentState);
bool kprocess_exit();
VMemoryManager* kprocess_getCurrentContext();
UINT kprocess_getCurrentPID();
