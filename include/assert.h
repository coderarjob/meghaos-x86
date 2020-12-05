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

#if defined(DEBUG)

/* If expression `t' is false, compiler will generate an error */
#define kstatic_assert(t) ((void)sizeof(char[2*!!(t) - 1]))

/* If assertion `t' is false, call kpanic() and halts. 
 * Displays message `e' in the panic message*/
#define kassert(t,e) ((t)) ? (void)0 \
                         : kpanic("Assertion failed:%s.\r\n%s", #t,e)
#else

/* These has no effect when DEBUG macro is not defined */
#define kstatic_assert(t) (void)0
#define kassert(t,e) (void)0 

#endif // defined(DEBUG)

#endif // __ASSERT_H__
