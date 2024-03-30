/*
 * --------------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Process management
 * --------------------------------------------------------------------------------
 */
#pragma once

#include <types.h>
#include <buildcheck.h>

void jump_to_usermode (U32 dataselector, U32 codeselector, void (*user_func)());
