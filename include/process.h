/*
 * --------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Process management
 * --------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#include <intrusive_queue.h>
#include <vmm.h>

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

typedef struct ProcessInfo {
    // Physical memorys for the process.
    struct {
        Physical PageDirectory;
        Physical Binary;
        Physical Stack;
        SIZE StackSizePages;
        SIZE BinarySizePages; // Only valid for non-thread processes.
    } physical;
    struct {
        PTR Entry;
        PTR StackStart;
    } virt;
    // ProcessInfos' are part of the scheduler process table through this node.
    ListNode schedulerQueueNode;
    // Process address spaces are managed through this VMM.
    VMManager* processVMM;
    // Initial states. These do not change throuout the lifetime of the process.
    UINT processID;
    ProcessFlags flags;
    // States which change
    ProcessStates state;
    ProcessRegisterState* registerStates;
} ProcessInfo;

void kprocess_init();
INT kprocess_create (void* processStartAddress, SIZE binLengthBytes, ProcessFlags flags);
bool kprocess_yield (ProcessRegisterState* currentState);
bool kprocess_exit();
