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
#pragma once

#include <buildcheck.h>
#include <types.h>

#ifndef UNITTEST
    void kpanic_ndu (UINT line, const CHAR* file, const CHAR* fmt, ...);

    /* Displays an error message on the screen and Halts */
    #define k_panic(...)                                  \
        do {                                              \
            ARCH_DISABLE_INTERRUPTS()                     \
            kpanic_ndu(__LINE__, __FILE__, __VA_ARGS__);  \
        } while (0)
#else // UNITTEST
void        unittest_panic_handler (const CHAR* s, ...);
extern bool panic_invoked;

    /* Returns from the 'function under testing', when an assert/panic is hit.
     *
     * There is x86 assembly hard coded in an arch independent header, however this corresponds to
     * the host (not the target) arch. Which implies that unittests can only be built & run on an
     * x86 machine.
     *
     * TODO: Find some way to make this host independent.
     * NOTE: EAX is not preserved by GCC. So there is not point adding it to the clobber list.
     */
    #define UNITTEST_RETURN() __asm__ volatile("mov esp, ebp; pop ebp; mov eax, 0; ret;" ::)

    #define k_panic(s, ...)                                                                \
        do                                                                                 \
        {                                                                                  \
            unittest_panic_handler ("\r\nPanic! " s ". In %s:%u", ##__VA_ARGS__, __func__, \
                                    __LINE__);                                             \
            UNITTEST_RETURN();                                                             \
        } while (0)
#endif // UNITTEST

/* Halts the processor by going into infinite loop */
#define k_halt()               \
    ARCH_DISABLE_INTERRUPTS(); \
    for (;;)
