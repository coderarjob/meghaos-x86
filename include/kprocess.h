/*
 * --------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Process management
 * --------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>

bool kprocess_create (void (*processStartAddress), SIZE binLengthBytes);
void kprocess_switch();
