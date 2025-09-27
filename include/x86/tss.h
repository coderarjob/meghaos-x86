/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - TSS header
* ---------------------------------------------------------------------------
*/

#ifndef TSS_H_X86
#define TSS_H_X86

#include <buildcheck.h>

/* Initializes the tss_entry structure, installs a tss segment in GDT */
void ktss_init  (void);

#endif // TSS_H_X86
