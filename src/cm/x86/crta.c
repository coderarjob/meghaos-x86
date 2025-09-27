/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - User Library - Application Prologue
 * -------------------------------------------------------------------------------------------------
 */

#include <types.h>
#include <cm/osif.h>
#include <cm/syscall.h>
#include <kernel.h>

// This is the entry point for all application processes.
__attribute__ ((section (".entry.text")))
void proc_start(void)
{
    __asm__("call proc_main;");
    while (1)
        ;
}
