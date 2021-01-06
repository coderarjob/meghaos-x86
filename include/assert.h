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

/* If expression `t' is false, compiler will generate an error 
 *
 * Note: This workes because an array dimension cannot be negative, which is
 * what the macro expands to when t == false.
 *
 * Note: sizeof(char[!!(t) - 1]) would also have worked, but when t == true, 
 * the array size will become 0. This gives compiler warning 
 * 'ISO C forbids zero sized array'. To avoid this warning/error, we multiply 
 * by 2.
 */
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
