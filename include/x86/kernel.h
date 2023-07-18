/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Contains function
* declerations and includes header files necessory for building x86 Kernel.
* These routines are not system calls and may not be accessible by user mode 
* programs.
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/

#ifndef KERNEL_H_X86
#define KERNEL_H_X86

#include <types.h>
#include <buildcheck.h>

void jump_to_usermode (U32 dataselector,
                       U32 codeselector, void (*user_func)());


#endif //KERNEL_H_x86
