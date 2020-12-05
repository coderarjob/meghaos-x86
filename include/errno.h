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
enum {ERR_NONE, ERR_UNKNOWN,ERRNO_COUNT};

/* This variable is globally used to set error codes*/
extern u32 kerrno;
/* Error descriptions indexed by kerrno */
extern char *kerrdes[];

/* Displays error description if kerrno != 0 */
#define kassert_perror() kassert(kerrno == ERR_NONE,kerrdes[kerrno])

#endif // __ERRORNO_H__
