/*
 * ----------------------------------------------------------------------------
 * Macros for equality and negation checks.
 *
 * Author: arjobmukherjee@gmail.com (Arjob Mukherjee)
 * Dated : 26 April 2022
 * ----------------------------------------------------------------------------
 */
#ifndef UNITTEST_H
#define UNITTEST_H
#include <stdio.h>
#include "fake.h"

int ut_equal_mem (const void *a, const void *b, unsigned long size, int *i);
int ut_equal_string (const char *a, const char *b, int *i);

#define COL_RED   "\x1b[31m"
#define COL_GREEN "\x1b[32m"
#define COL_RESET "\x1b[0m"

#define UT_PASSED(t) printf ("\n  %sPass%s: %-20s", COL_GREEN, COL_RESET, #t)

#define UT_FAILED(t, fnt, ...)                                                \
    do {                                                                      \
         printf ("\n  %s** FAIL ** %s: %-20s: ", COL_RED, COL_RESET, #t);     \
         printf(fnt, __VA_ARGS__);                                            \
        } while(0)

#define TEST_SCALAR(a,o, b)                                                   \
    if (a o b) UT_PASSED(a o b);                                              \
    else       UT_FAILED(a o b,"[Got: %d, Exp: %d]", a, b)                    \

#define TEST_MEM(a,o, b,sz) do {                                              \
    int i;                                                                    \
    if (ut_equal_mem (a, b, sz, &i) o 1)                                      \
           UT_PASSED(a o b);                                                  \
    else   UT_FAILED(a o b,"[Idx: %d, Got: %XH, Exp: %s %XH]",i,a[i],#o,b[i]);\
} while(0)

#define TEST_STRING(a,o, b) do {                                              \
    int i;                                                                    \
    if (ut_equal_string (a, b, &i) o 1)                                       \
           UT_PASSED(a o b);                                                  \
    else   UT_FAILED(a o b,"[Idx: %d, Got: %c, Exp: %s %c]",i,a[i],#o,b[i]);  \
} while(0)

#define EQ_SCALAR(a,b)  TEST_SCALAR(a, ==, b)
#define NEQ_SCALAR(a,b) TEST_SCALAR(a, !=, b)
#define GEQ_SCALAR(a,b) TEST_SCALAR(a, >=, b)
#define LEQ_SCALAR(a,b) TEST_SCALAR(a, <=, b)
#define LES_SCALAR(a,b) TEST_SCALAR(a, <, b)
#define GRT_SCALAR(a,b) TEST_SCALAR(a, >, b)

#define EQ_MEM(a,b,sz)  TEST_MEM(a, ==, b, sz)
#define NEQ_MEM(a,b,sz) TEST_MEM(a, !=, b, sz)

#define EQ_STRING(a,b)  TEST_STRING(a, ==, b)
#define NEQ_STRING(a,b) TEST_STRING(a, !=, b)

#define TEST(tf, fn) static void fn () {                                      \
                        reset();                                              \
                        printf ("TEST (%s) %s", #tf, #fn); do
#define END()       } while(0); printf("\n")
#endif // UNITTEST_H
