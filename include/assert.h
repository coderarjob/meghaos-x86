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

#define KASSERT_NEVER_REACH() kassert(0)

#define kassert(t) ((t)) ? (void)0 \
                         : kpanic("Assertion failed: %s", #t)

#endif // __ASSERT_H__
