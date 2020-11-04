/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - kassert() and kpanic() 
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 5st November 2020
*/
#ifndef __ASSERT_H__
#define __ASSERT_H__

#define KASSERT_NEVER_REACH() kassert(0,"SHOULD NEVER REACH HERE")

/* If expression `t' is false, compiler will generate an error */
#define kstatic_assert(t) ((void)sizeof(char[2*!!(t) - 1]))

/* If assertion `t' is false, call kpanic() and halts. 
 * Displays message `e' in the panic message*/
#define kassert(t,e) ((t)) ? (void)0 \
                         : kpanic("Assertion failed:%s. Message:" e, #t)

#endif // __ASSERT_H__
