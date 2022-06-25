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

/* Error codes that can be set inside the kernel.*/
typedef enum KernelErrorCodes 
{
    ERR_NONE            =    0,
    ERR_UNKNOWN,
    ERR_INVALID_RANGE,
    ERR_OVERFLOW,
    ERR_PMM_OUT_OF_MEM,
} KernelErrorCodes;


/* This variable is globally used to set error codes*/
extern KernelErrorCodes k_errorNumber;
/* Error descriptions indexed by k_errorNumber */
extern CHAR *k_errorText[];

/* Can be used to set the k_errorNumber global and return from a function */
#define RETURN_ERROR(erno, rval) do {                           \
                                        k_errorNumber = erno;   \
                                        return rval;            \
                                 }while(0)

/* Displays error description if k_errorNumber != 0 */
#define k_assertOnError() k_assert (k_errorNumber == ERR_NONE,k_errorText[k_errorNumber])

#endif // ERRORNO_H
