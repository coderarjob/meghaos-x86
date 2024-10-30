/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - User Library - Application Prologue
 * -------------------------------------------------------------------------------------------------
 */

#include <types.h>
#include <applib/osif.h>
#include <applib/syscall.h>
#include <kernel.h>

// This is the entry point for all application processes.
__attribute__ ((section (".entry.text")))
void proc_start()
{
    __asm__("jmp proc_main;");
    while (1)
        ;
}
