/*
 * Fake function macros for Unittesting parts of a program.
 * These macros make it easy to create fake/mock functions, with support for
 * return and byref parameters.
 *
 * Author: arjobmukherjee@gmail.com (Arjob Mukherjee)
 * Dated : 26 April 2022
 * ----------------------------------------------------------------------------
 * Examples:
 *
 * C declaration              Fake declaration and defination
 * ------------------------   -----------------------------------------
 * void f ();                 DECLARE_FUNC_VOID(f);
 *                            DEFINE_FUNC_VOID(f);
 *
 * void f (int a);            DECLARE_FUNC_VOID(f, int);
 *                            DEFINE_FUNC_VOID(f, int);
 *
 * int f ();                  DECLARE_FUNC(int, f);
 *                            DEFINE_FUNC(int, f);
 *
 * int f (int a);             DECLARE_FUNC(int, f, int);
 *                            DEFINE_FUNC(int, f, int);
 * ----------------------------------------------------------------------------
 */

#ifndef UNITTESTS_FAKE
#define UNITTESTS_FAKE
#include <string.h>

void reset();          // MUST BE DEFINED BY THE USER OF fake.h

// ----------------------------------------------------------------------------
// Common Helper macros used by both declaration and defination macros.
// ----------------------------------------------------------------------------
#define FK_STRUCT_TAG(f) f ## _fake_tag
#define FK_STRUCT_VAR(f) f ## _fake
#define FK_STRUCT_HANDLER(f) f ## _fake_handler

// ----------------------------------------------------------------------------
// Macros to find out the number of arguments passed to a variarg macros
// ----------------------------------------------------------------------------
#define FK_FIFTEENTH_ELEMENT(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,...) a15
#define FK_COUNT_ARGS(...) \
    FK_FIFTEENTH_ELEMENT(dummy, ##__VA_ARGS__, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// =====================FAKE FUNCTION DEFINATION ]=============================
#define DEFINE_FUNC_VOID(f, ...)                \
    FK_DEFINE_FUNC_STRUCT(f);                   \
    FK_DEFINE_FUNC_BODY_VOID(FK_COUNT_ARGS(__VA_ARGS__), f, __VA_ARGS__)

// ----
#define DEFINE_FUNC(rt, f, ...)                 \
    FK_DEFINE_FUNC_STRUCT(f);                   \
    FK_DEFINE_FUNC_BODY(FK_COUNT_ARGS(__VA_ARGS__), rt, f, __VA_ARGS__)

// ----
#define FK_DEFINE_FUNC_STRUCT(f)  FK_STRUCT_TAG(f) FK_STRUCT_VAR(f) = {}

#define FK_DEFINE_FUNC_BODY_VOID(n, f, ...)                                    \
  void f(FK_FUNC_PARAMS_X(n, __VA_ARGS__)) {                                   \
    FK_STRUCT_VAR(f).invokeCount++;                                            \
    FK_RETURN_VOID(n, f, __VA_ARGS__);                                         \
  }

#define FK_DEFINE_FUNC_BODY(n, rt, f, ...)                                     \
  rt f(FK_FUNC_PARAMS_X(n, __VA_ARGS__)) {                                     \
    FK_STRUCT_VAR(f).invokeCount++;                                            \
    FK_RETURN(n, f, __VA_ARGS__);                                              \
  }

#define FK_RETURN_VOID(n,f,...)                        \
    if (FK_STRUCT_VAR(f).handler)                      \
        FK_STRUCT_VAR(f).handler(FK_FUNC_ARG_ ## n())  \

#define FK_RETURN(n,f,...)                                  \
    return (FK_STRUCT_VAR(f).handler)                       \
           ? FK_STRUCT_VAR(f).handler(FK_FUNC_ARG_ ## n())  \
           : FK_STRUCT_VAR(f).ret

#define FK_FUNC_PARAMS_X(n,...) FK_FUNC_PARAMS_ ## n(__VA_ARGS__)
#define FK_FUNC_PARAMS_4(t,...) t d, FK_FUNC_PARAMS_3(__VA_ARGS__)
#define FK_FUNC_PARAMS_3(t,...) t c, FK_FUNC_PARAMS_2(__VA_ARGS__)
#define FK_FUNC_PARAMS_2(t,...) t b, FK_FUNC_PARAMS_1(__VA_ARGS__)
#define FK_FUNC_PARAMS_1(t,...) t a
#define FK_FUNC_PARAMS_0(...)

#define FK_FUNC_ARG_4() d, FK_FUNC_PARAMS_3()
#define FK_FUNC_ARG_3() c, FK_FUNC_PARAMS_2()
#define FK_FUNC_ARG_2() b, FK_FUNC_PARAMS_1()
#define FK_FUNC_ARG_1() a
#define FK_FUNC_ARG_0()

// =======================[ FAKE FUNCTION DECLARATION ]========================
#define DECLARE_FUNC_VOID(f, ...)                   \
    typedef void (*f##_fake_handler) (__VA_ARGS__); \
    FK_DECLARE_STRUCT_VOID (f)

// ----
#define DECLARE_FUNC(rt, f, ...)                  \
    typedef rt (*f##_fake_handler) (__VA_ARGS__); \
    FK_DECLARE_STRUCT (rt, f)

// ----
#define FK_DECLARE_STRUCT_VOID(f)                         \
    FK_DECLARE_STRUCT_START (f)                           \
        FK_STRUCT_FIELD (void*, resources);               \
        FK_STRUCT_FIELD (unsigned int, invokeCount);      \
        FK_STRUCT_FIELD (FK_STRUCT_HANDLER (f), handler); \
    FK_DECLARE_STRUCT_END (f)

#define FK_DECLARE_STRUCT(rt, f)                          \
    FK_DECLARE_STRUCT_START (f)                           \
        FK_STRUCT_FIELD (void*, resources);               \
        FK_STRUCT_FIELD (unsigned int, invokeCount);      \
        FK_STRUCT_FIELD (rt, ret);                        \
        FK_STRUCT_FIELD (FK_STRUCT_HANDLER (f), handler); \
    FK_DECLARE_STRUCT_END (f)

#define FK_DECLARE_STRUCT_START(f)  typedef struct FK_STRUCT_TAG(f) {
#define FK_STRUCT_FIELD(pt, p)              pt p;
#define FK_DECLARE_STRUCT_END(f)  } FK_STRUCT_TAG(f);                         \
                                  extern FK_STRUCT_TAG(f) FK_STRUCT_VAR(f)

// =======================[ RESET FAKE ]========================
#define RESET_FAKE(f) memset(&FK_STRUCT_VAR(f), 0, sizeof(FK_STRUCT_VAR(f)))

#endif
