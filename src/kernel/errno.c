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
KernelErrorCodes k_errorNumber;

/* Error descriptions indexed by k_errorNumber */
/* NOTE: In a measure to reduce the kernel binary, we can only create the 
 * below array if DEBUG is defined */
#if defined (DEBUG)
CHAR *k_errorText[] = {
                    "No error",
                    "Unknown error"
                 };
#endif
