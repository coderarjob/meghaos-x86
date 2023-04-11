/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - k_panic () 
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 5st November 2020
*/
#ifndef KPANIC_H
#define KPANIC_H

/* Displays an error message on the screen and Halts */
#ifdef __C99__
#define k_panic(s,...) k_panic_ndu ("\r\nKernel Panic!\r\n" s "\r\nin %s:%u:%s" \
                               ,__VA_ARGS__, __FILE__,__LINE__,__func__)
#else
#define k_panic(s,...) k_panic_ndu ("\r\nKernel Panic!\r\n" s "\r\nin %s:%u" \
                               ,__VA_ARGS__, __FILE__,__LINE__)
#endif // __C99__

/* Displays an error message on the screen and Halts 
 * Note: The attribute here suppresses warning from GCC when used within
 * another `noreturn` function.
 * */
__attribute__ ((noreturn))
void k_panic_ndu (const CHAR *s,...);

#endif // KPANIC_H
