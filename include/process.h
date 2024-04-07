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
    PROCESS_FLAGS_KERNEL_PROCESS = (1 << 0),
} ProcessFlags;

// TODO: Fields to store registers and return address is missing at this point.
typedef struct ProcessInfo {
    INT processID;
    Physical pagedir;
    ProcessStates state;
    Physical binaryAddress;
    Physical stackAddress;
    ProcessFlags flags;
} ProcessInfo;

INT kprocess_create (void* processStartAddress, SIZE binLengthBytes, ProcessFlags flags);
bool kprocess_switch (UINT processID);
