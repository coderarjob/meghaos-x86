/*
 * --------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Process management
 * --------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#include <intrusive_queue.h>
#include <intrusive_list.h>
#include <vmm.h>

#define PROCESS_ID_KERNEL  0x0
#define PROCESS_ID_INVALID -1

typedef enum KProcessStates {
    PROCESS_STATE_INVALID = 0,
    PROCESS_STATE_RUNNING = 1,
    PROCESS_STATE_IDLE    = 2,
} KProcessStates;

typedef enum KProcessFlags {
    PROCESS_FLAGS_NONE           = 0,
    PROCESS_FLAGS_KERNEL_PROCESS = (1 << 0),
    PROCESS_FLAGS_THREAD         = (1 << 1),
} KProcessFlags;

__asm__(".equ PROCESS_FLAGS_KERNEL_PROCESS, (1 << 0);"
        ".equ PROCESS_FLAGS_THREAD,         (1 << 1);");

typedef struct KProcessRegisterState ProcessRegisterState;

typedef struct KProcessSections {
    PTR virtualMemoryStart;
    SIZE sizePages;
} KProcessSections;

typedef struct KProcessEvent {
    UINT event;
    UINT data;
    ListNode eventQueueNode;
} KProcessEvent;

typedef struct KProcessInfo {
    // ----------------------
    // Initial states. These do not change throuout the lifetime of the process.
    // ----------------------
    KProcessSections binary;
    KProcessSections stack;
    KProcessSections data;
    VMemoryManager* context;
    ListNode schedulerQueueNode; // Processes are part of scheduler queue through this node.
    ListNode eventsQueueHead;    // Start of the process events queue.
    UINT processID;
    KProcessFlags flags;
    // ----------------------
    // States which change
    // ----------------------
    KProcessStates state;
    ProcessRegisterState* registerStates;
} KProcessInfo;

void kprocess_init();
INT kprocess_create (void* processStartAddress, SIZE binLengthBytes, KProcessFlags flags);
bool kprocess_yield (ProcessRegisterState* currentState);
bool kprocess_exit();
VMemoryManager* kprocess_getCurrentContext();
UINT kprocess_getCurrentPID();
bool kprocess_popEvent (UINT pid, KProcessEvent* ev);
bool kprocess_pushEvent (UINT pid, UINT eventID, UINT eventData);
