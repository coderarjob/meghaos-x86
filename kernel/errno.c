/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - errno and error reporting
* routines.
* ---------------------------------------------------------------------------
*
* Dated: 9th November 2020
*/

#include <kernel.h>

/* This variable is globally used to set error codes*/
u32 kerrno;

/* Error descriptions indexed by kerrno */
char *kerrdes[] = {
                    "No error",
                    "Unknown error"
                 };
