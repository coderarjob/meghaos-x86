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
#include <disp.h>

#ifndef UNITTEST
    /* Displays an error message on the screen and Halts */
    #define k_panic(s, ...) do {                                                                   \
        kdisp_importantPrint("\r\nKernel Panic!\r\n" s "\r\nin %s:%u:%s", __VA_ARGS__,             \
                    __FILE__, __LINE__, __func__);                                                 \
        k_halt();                                                                                  \
    } while(0)
#else      // UNITTEST
extern bool panic_invoked;
    #define k_panic(s, ...)                                                                        \
        k_panic_ndu("\r\nPanic! " s ". In %s:%u", __VA_ARGS__, __func__, __LINE__)
#endif // UNITTEST


/* Halts the processor by going into infinite loop */
#define k_halt() for (;;)

#endif // KPANIC_H
