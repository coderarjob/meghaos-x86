/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - kpanic() 
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 5st November 2020
*/
#ifndef __KPANIC_H__
#define __KPANIC_H__

/* Displays an error message on the screen and Halts */
#ifdef __C99__
#define kpanic(s,...) __kpanic(s "\r\nin %s:%d:%s", __VA_ARGS__, \
                               __FILE__,__LINE__,__func__)
#else
#define kpanic(s,...) __kpanic(s "\r\nin %s:%d", __VA_ARGS__, \
                               __FILE__, __LINE__)
#endif // __C99__

/* Displays an error message on the screen and Halts */
void __kpanic(const char *s,...);

#endif // __KPANIC_H__
