/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Common stdlib functions
* ---------------------------------------------------------------------------
*/

#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <types.h>
#include <buildcheck.h>

/* Copies n bytes from src to dest. Returns dest */
void *k_memcpy (void *dest, const void *src, size_t n);

/* Fills memory with constant byte */
void *k_memset (void *s, INT c, size_t n);

#endif //MEM_H
