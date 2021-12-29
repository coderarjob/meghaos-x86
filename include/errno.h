/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - errno and error reporting
* routines.
* ---------------------------------------------------------------------------
*
* Dated: 9th November 2020
*/

#ifndef __ERRORNO_H__
#define __ERRORNO_H__

/* Error codes that can be set inside the kernel.*/
typedef enum KernelErrorCodes 
{
    ERR_NONE, 
    ERR_UNKNOWN,
    ERRNO_COUNT
} KernelErrorCodes;

/* This variable is globally used to set error codes*/
extern KernelErrorCodes k_errorNumber;
/* Error descriptions indexed by k_errorNumber */
extern CHAR *k_errorText[];

/* Displays error description if k_errorNumber != 0 */
#define k_assertOnError() k_assert (k_errorNumber == ERR_NONE,k_errorText[k_errorNumber])

#endif // __ERRORNO_H__
