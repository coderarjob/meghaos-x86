/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - k_assert ()
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 5st November 2020
*/
#ifndef KASSERT_H
#define KASSERT_H

#include <buildcheck.h>

#if defined(DEBUG)
    #include <panic.h>
    /* If assertion `t' is false, call k_panic () and halts.
     * Displays message `e' in the panic message.
     */
    #define k_assert(t, e)                                                         \
        do {                                                                       \
            if (!(t))                                                              \
                k_panic ("Assertion failed: %s\nAt %s:%u", e, __FILE__, __LINE__); \
        } while (0)

    /* If expression `t' is false, compiler will generate an error.
     *
     * This works because an array dimension cannot be negative, which is what the macro expands to
     * when t == false.
     */
    #define CALL_COMPILER_CHECK2_NDU(m, t) \
        __attribute__ ((unused)) typedef char static_assert_##m[(t) ? 1 : -1]

    #define CALL_COMPILER_CHECK_NDU(m, t) CALL_COMPILER_CHECK2_NDU (m, t)

    #define k_staticAssert(t)             CALL_COMPILER_CHECK_NDU (__LINE__, t)

#else
    #define k_assert(t, e) (void)0
    #define k_staticAssert(t) (void)0
#endif // DEBUG

#endif // KASSERT_H
