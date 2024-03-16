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
#ifndef ASSERT_H
#define ASSERT_H

#include <buildcheck.h>

#if defined(DEBUG) || defined (UNITTEST)
    #include <panic.h>
    /* If assertion `t' is false, call k_panic () and halts.
     * Displays message `e' in the panic message.
     */
    #define k_assert(t, e)                                                                 \
        do                                                                                 \
        {                                                                                  \
            if (!(t))                                                                      \
                k_panic ("Assertion failed:%s.\n%s\nAt %s:%u", #t, e, __FILE__, __LINE__); \
        } while (0)

    /* If expression `t' is false, compiler will generate an error
     *
     * Note: This works because an array dimension cannot be negative, which is
     * what the macro expands to when t == false.
     *
     * Note: sizeof (CHAR[!!(t) - 1]) would also have worked, but when t == true,
     * the array size will become 0. This gives compiler warning
     * 'ISO C forbids zero sized array'. To avoid this warning/error, we multiply
     * by 2.
     */
    #define k_staticAssert(t) ((void)sizeof(CHAR[2 * !!(t)-1]))
#else
    #define k_assert(t, e) (void)0
    #define k_staticAssert(t) (void)0
#endif // DEBUG

#endif // ASSERT_H
