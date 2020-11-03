/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Memory functions
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
* Dated: 1st November 2020
*
; Bulid 20201101
; - Initial version 
; ---------------------------------------------------------------------------
*/

#ifndef __MEM_H__
#define __MEM_H__

#ifdef __i386__
    #include <x86/memloc.h>
#endif

/* Copies n bytes from src to dest. Returns dest */
void *memcpy(void *dest, const void *src, size_t n);

/* Fills memory with constant byte */
void *memset(void *s, int c, size_t n);

#endif //__MEM_H__
