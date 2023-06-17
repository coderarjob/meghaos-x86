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

#include <buildcheck.h>
#include <types.h>

#ifndef UNITTEST
    /* Displays an error message on the screen and Halts */
    #ifdef __C99__
        #define k_panic(s, ...)                                                                    \
            k_panic_ndu("\r\nKernel Panic!\r\n" s "\r\nin %s:%u:%s", __VA_ARGS__, __FILE__,        \
                        __LINE__, __func__)
    #else // __C99__
        #define k_panic(s, ...)                                                                    \
            k_panic_ndu("\r\nKernel Panic!\r\n" s "\r\nin %s:%u", __VA_ARGS__, __FILE__, __LINE__)
    #endif // __C99__
#else      // UNITTEST
extern bool panic_invoked;
    #define k_panic(s, ...)                                                                        \
        k_panic_ndu("\r\nPanic! " s ". In %s:%u", __VA_ARGS__, __func__, __LINE__)
#endif // UNITTEST

/* Displays an error message on the screen and Halts */
#ifndef UNITTEST
/* Note: The attribute here suppresses warning from GCC when used within
 * another `noreturn` function.
 *
 * Note: In an unittest, this function have to return (otherwise test will hung up), so this
 * attribute is not applied when building unittests. Also causes weird effects if a function with
 * 'noreturn' attribute returns.
 */
__attribute__ ((noreturn))
#endif // UNITTEST
void k_panic_ndu (const CHAR *s,...);

/* Halts the processor by going into infinite loop */
#define k_halt() for (;;)

#endif // KPANIC_H
