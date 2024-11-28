/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - App library - General macros & function declarations
 * ---------------------------------------------------------------------------
 *
 *  This file contains Mos-libc macros & definitions for application to use.
 */

#pragma once

#include <types.h>
#include <stdarg.h>

#define INVALID_HANDLE (-1)
#define HALT()         for (;;)

INT snprintf (CHAR* dest, size_t size, const CHAR* fmt, ...);
INT vsnprintf (CHAR* dest, size_t size, const CHAR* fmt, va_list l);

/***************************************************************************************************
 * Halts thread for 'ms' miliseconds
 *
 * @return      Nothing
 **************************************************************************************************/
void cm_delay (UINT ms);
