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
        kdisp_show_call_trace();                                                                   \
        k_halt();                                                                                  \
    } while(0)
#else      // UNITTEST
    void unittest_panic_handler(const CHAR *s,...);
    extern bool panic_invoked;

    /* Returns from the 'function under testing', when an assert/panic is hit.
     *
     * There is x86 assembly hard coded in an arch independent header, however this corresponds to
     * the host (not the target) arch. Which implies that unittests can only be built & run on an
     * x86 machine.
     *
     * TODO: Find some way to make this host independent.
     */
    #define UNITTEST_RETURN() __asm__ volatile ("mov esp, ebp; pop ebp; ret;"::)

    #define k_panic(s, ...)  do {                                                                  \
        unittest_panic_handler("\r\nPanic! " s ". In %s:%u", __VA_ARGS__, __func__, __LINE__);     \
        UNITTEST_RETURN();                                                                         \
    } while (0)
#endif // UNITTEST

/* Halts the processor by going into infinite loop */
#define k_halt() for (;;)

#endif // KPANIC_H
