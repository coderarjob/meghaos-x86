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

#ifndef __KERNEL_H_x86__
#define __KERNEL_H_x86__

#include <x86/boot.h>
#include <x86/paging.h>
#include <x86/gdt.h>
#include <x86/idt.h>

/* Initializes the tss_entry structure, installs a tss segment in GDT */
void ktss_init  ();

#endif //__KERNEL_H_x86__
