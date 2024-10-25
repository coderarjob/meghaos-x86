/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Kernel Object Handle management headers
 *
 * Handles identify objects in the kernel and can safely be passed to applications as well.
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#include <kerror.h>

#define INVALID_HANLDE KERNEL_EXIT_FAILURE

void khandle_init();
Handle khandle_createHandle (void* obj);
bool khandle_freeHandle (Handle h);
void* khandle_getObject (Handle h);
