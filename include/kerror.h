/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - errno and error reporting
* routines.
* ---------------------------------------------------------------------------
*
* Dated: 9th November 2020
*/

#ifndef ERRORNO_H
#define ERRORNO_H

#include <types.h>
#include <kassert.h>
#include <kdebug.h>

/* Error codes that can be set inside the kernel.*/
typedef enum KernelErrorCodes 
{
    ERR_NONE                        =    0,
    ERR_UNKNOWN                     =    1,
    ERR_INVALID_RANGE               =    2,
    ERR_OVERFLOW                    =    3,
    ERR_OUT_OF_MEM                  =    4,
    ERR_DOUBLE_FREE                 =    5,
    ERR_DOUBLE_ALLOC                =    6,
    ERR_WRONG_ALIGNMENT             =    7,
    ERR_OUTSIDE_ADDRESSABLE_RANGE   =    8,
    ERR_INVALID_ARGUMENT            =    9
} KernelErrorCodes;

/* This variable is globally used to set error codes*/
extern KernelErrorCodes k_errorNumber;
/* Error descriptions indexed by k_errorNumber */
extern CHAR *k_errorText[];

/* Can be used to set the k_errorNumber global and return from a function */
#define RETURN_ERROR(errno, rval)                                                            \
            do {                                                                             \
                   ERROR ("Error 0x%x.", errno);                                             \
                   k_errorNumber = errno;                                                    \
                   return rval;                                                              \
                }while(0)

#define KERNEL_EXIT_SUCCESS  0
#define KERNEL_EXIT_FAILURE -1

/* Displays error description if k_errorNumber != 0 */
#define k_assertOnError() k_assert (k_errorNumber == ERR_NONE,k_errorText[k_errorNumber])

#endif // ERRORNO_H
