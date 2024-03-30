/*
 * --------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Process management
 * --------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>

typedef enum ProcessStates
{
    PROCESS_NOT_CREATED,
    PROCESS_NOT_STARTED,
    PROCESS_STARTED,
    PROCESS_ENDED
} ProcessStates;

typedef struct ProcessInfo
{
    Physical pagedir;
    ProcessStates state;
    Physical bin_addr;
    Physical stack_addr;
} ProcessInfo;

INT kprocess_create (void (*processStartAddress), SIZE binLengthBytes);
bool kprocess_switch(INT processID);
