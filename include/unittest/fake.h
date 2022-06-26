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
 * C declaration              Fake declaration
 * ------------------------   -----------------------------------------
 * void f ();                 DECLARE_FUNC_VOID_0(f);
 * void f (int a);            DECLARE_FUNC_VOID_1(f, IN, int);
 * void f (int *a);           DECLARE_FUNC_VOID_1(f, IN, int*);
 * void f (int *a);           DECLARE_FUNC_VOID_1(f, OUT, int*);
 * void f (int a, char* b);   DECLARE_FUNC_VOID_2(f, IN, int, *, OUT, char*);
 *
 * int f ();                  DECLARE_FUNC_0(int, f);
 * int f (int a);             DECLARE_FUNC_1(int, f, IN, int);
 * int f (int *a);            DECLARE_FUNC_1(int, f, IN, int*);
 * int f (int *a);            DECLARE_FUNC_1(int, f, OUT, int*);
 * int f (int a, char* b);    DECLARE_FUNC_2(int, f, IN, int, *, OUT, char*);
 * ----------------------------------------------------------------------------
 */

#ifndef UNITTESTS_FAKE
#define UNITTESTS_FAKE
#include <string.h>

void reset();          // MUST BE DEFINED BY THE USER OF fake.h

/*
 * -------------------------------------
 * Macro param nomenclature
 * Param   Meaning
 * ------  -----------------------------
 * f       function name
 * pt      param type
 * pd      param direction (input/output)
 * p       parameter name
 * rt      FK_RETURN type
 * -------------------------------------
 */

// ----------------------------------------------------------------------------
// Common Helper macros used by both declaration and defination macros.
// ----------------------------------------------------------------------------
#define FK_STRUCT_TAG(f) f ## _fake_tag
#define FK_STRUCT_VAR(f) f ## _fake
#define OUT   OUT
#define IN    IN

// =====================FAKE FUNCTION DEFINATION ]=============================
// ----------------------------------------------------------------------------
// Macros to craete fake function declarations
// ----------------------------------------------------------------------------
#define FK_DEFINE_FUNC_BODY_0(rt, f)                rt f()
#define FK_DEFINE_FUNC_BODY_1(rt, f, pt1)           rt f(pt1 a)
#define FK_DEFINE_FUNC_BODY_2(rt, f, pt1, pt2)      rt f(pt1 a, pt2 b)
#define FK_DEFINE_FUNC_BODY_3(rt, f, pt1, pt2, pt3) rt f(pt1 a, pt2 b, pt3 c)
// ----------------------------------------------------------------------------
// Helper Macros
// ----------------------------------------------------------------------------
#define FK_DEFINE_FUNC_STRUCT(f)      FK_STRUCT_TAG(f) FK_STRUCT_VAR(f) = {}
#define FK_INCREMENT_INVOKE_COUNT(f)  FK_STRUCT_VAR(f).invoke_count++;
// ------
#define FK_PARAM_OUT_SET(f, p)                                                \
    *p = (FK_STRUCT_VAR(f).type == STATIC)                                    \
         ? FK_STRUCT_VAR(f).p[0]  /* Output the first element. */             \
         : FK_STRUCT_VAR(f).p[FK_STRUCT_VAR(f).invoke_count]

#define FK_PARAM_IN_SET(f, p)  /* No need for a field for input param */

#define FK_PARAM_SET(f, pd, p) FK_PARAM_ ## pd ## _SET(f, p)
// ------
// If Static, output the first element or NULL.
#define FK_RETURN(f)                                                          \
    return FK_STRUCT_VAR(f).ret
// ----------------------------------------------------------------------------
// Complete fake function definations
// ----------------------------------------------------------------------------
#define DEFINE_FUNC_VOID_0(f)                                                 \
    FK_DEFINE_FUNC_STRUCT(f);                                                 \
    FK_DEFINE_FUNC_BODY_0(void, f) {                                          \
        FK_INCREMENT_INVOKE_COUNT(f);                                         \
    }
// ------
#define DEFINE_FUNC_VOID_1(f, pd1, pt1)                                       \
    FK_DEFINE_FUNC_STRUCT(f);                                                 \
    FK_DEFINE_FUNC_BODY_1(void, f, pt1) {                                     \
        FK_PARAM_SET(f, pd1, a);                                              \
        FK_INCREMENT_INVOKE_COUNT(f);                                         \
    }
// ------
#define DEFINE_FUNC_VOID_2(f, pd1, pt1, pd2, pt2)                             \
    FK_DEFINE_FUNC_STRUCT(f);                                                 \
    FK_DEFINE_FUNC_BODY_2(void, f, pt1, pt2) {                                \
        FK_PARAM_SET(f, pd1, a);                                              \
        FK_PARAM_SET(f, pd2, b);                                              \
        FK_INCREMENT_INVOKE_COUNT(f);                                         \
    }
// ------
#define DEFINE_FUNC_VOID_3(f, pd1, pt1, pd2, pt2, pd3, pt3)                   \
    FK_DEFINE_FUNC_STRUCT(f);                                                 \
    FK_DEFINE_FUNC_BODY_3(void, f, pt1, pt2, pt3) {                           \
        FK_PARAM_SET(f, pd1, a);                                              \
        FK_PARAM_SET(f, pd2, b);                                              \
        FK_PARAM_SET(f, pd3, c);                                              \
        FK_INCREMENT_INVOKE_COUNT(f);                                         \
    }
// ----------------------------------------------------------------------------
#define DEFINE_FUNC_0(rt, f)                                                  \
    FK_DEFINE_FUNC_STRUCT(f);                                                 \
    FK_DEFINE_FUNC_BODY_0(rt, f) {                                            \
        FK_INCREMENT_INVOKE_COUNT(f);                                         \
        FK_RETURN(f);                                                         \
    }
// ------
#define DEFINE_FUNC_1(rt, f, pd1, pt1)                                        \
    FK_DEFINE_FUNC_STRUCT(f);                                                 \
    FK_DEFINE_FUNC_BODY_1(rt, f, pt1) {                                       \
        FK_PARAM_SET(f, pd1, a);                                              \
        FK_INCREMENT_INVOKE_COUNT(f);                                         \
        FK_RETURN(f);                                                         \
    }
// ------
#define DEFINE_FUNC_2(rt, f, pd1, pt1, pd2, pt2)                              \
    FK_DEFINE_FUNC_STRUCT(f);                                                 \
    FK_DEFINE_FUNC_BODY_2(rt, f, pt1, pt2) {                                  \
        FK_PARAM_SET(f, pd1, a);                                              \
        FK_PARAM_SET(f, pd2, b);                                              \
        FK_INCREMENT_INVOKE_COUNT(f);                                         \
        FK_RETURN(f);                                                         \
    }
// ------
#define DEFINE_FUNC_3(rt, f, pd1, pt1, pd2, pt2, pd3, pt3)                    \
    FK_DEFINE_FUNC_STRUCT(f);                                                 \
    FK_DEFINE_FUNC_BODY_3(rt, f, pt1, pt2, pt3) {                             \
        FK_PARAM_SET(f, pd1, a);                                              \
        FK_PARAM_SET(f, pd2, b);                                              \
        FK_PARAM_SET(f, pd3, c);                                              \
        FK_INCREMENT_INVOKE_COUNT(f);                                         \
        FK_RETURN(f);                                                         \
    }

// =======================[ FAKE FUNCTION DECLARATION ]========================
// ----------------------------------------------------------------------------
// Helper macros
// ----------------------------------------------------------------------------
#define FK_DECLARE_STRUCT_START(f)  typedef struct FK_STRUCT_TAG(f) {         \
                                        int invoke_count;

// -----
#define FK_DECLARE_STRUCT_END(f)  } FK_STRUCT_TAG(f);                         \
                                   extern FK_STRUCT_TAG(f) FK_STRUCT_VAR(f)
// ------
#define FK_STRUCT_OUT_FIELD(pt, p)  pt p
#define FK_STRUCT_IN_FIELD(pt, p) /* No need for a field for input param */

#define FK_STRUCT_FIELD(pd, pt, p) FK_STRUCT_## pd ## _FIELD(pt, p)
// ----------------------------------------------------------------------------
// Reset function structure
// ----------------------------------------------------------------------------
#define RESET_FAKE(f) memset(&FK_STRUCT_VAR(f), 0, sizeof(FK_STRUCT_VAR(f)))

// ----------------------------------------------------------------------------
// Macros to define fake function structure for output and return parameters.
// ----------------------------------------------------------------------------
#define FK_DECLARE_STRUCT_VOID_0(f)                                           \
    FK_DECLARE_STRUCT_START(f);                                               \
    FK_DECLARE_STRUCT_END(f)
// ------
#define FK_DECLARE_STRUCT_VOID_1(f, pd1, pt1)                                 \
    FK_DECLARE_STRUCT_START(f);                                               \
        FK_STRUCT_FIELD(pd1, pt1, a);                                         \
    FK_DECLARE_STRUCT_END(f)
// ------
#define FK_DECLARE_STRUCT_VOID_2(f, pd1, pt1, pd2, pt2)                       \
    FK_DECLARE_STRUCT_START(f);                                               \
        FK_STRUCT_FIELD(pd1, pt1, a);                                         \
        FK_STRUCT_FIELD(pd2, pt2, b);                                         \
    FK_DECLARE_STRUCT_END(f)
// ------
#define FK_DECLARE_STRUCT_VOID_3(f, pd1, pt1, pd2, pt2, pd3, pt3)             \
    FK_DECLARE_STRUCT_START(f);                                               \
        FK_STRUCT_FIELD(pd1, pt1, a);                                         \
        FK_STRUCT_FIELD(pd2, pt2, b);                                         \
        FK_STRUCT_FIELD(pd3, pt3, c);                                         \
    FK_DECLARE_STRUCT_END(f)
// ----------------------------------------------------------------------------
#define FK_DECLARE_STRUCT_0(rt, f)                                            \
    FK_DECLARE_STRUCT_START(f);                                               \
        FK_STRUCT_FIELD(OUT, rt, ret);                                        \
    FK_DECLARE_STRUCT_END(f)
// ------
#define FK_DECLARE_STRUCT_1(rt, f, pd1, pt1)                                  \
    FK_DECLARE_STRUCT_START(f);                                               \
        FK_STRUCT_FIELD(OUT, rt, ret);                                        \
        FK_STRUCT_FIELD(pd1, pt1, a);                                         \
    FK_DECLARE_STRUCT_END(f)
// ------
#define FK_DECLARE_STRUCT_2(rt, f, pd1, pt1, pd2, pt2)                        \
    FK_DECLARE_STRUCT_START(f);                                               \
        FK_STRUCT_FIELD(OUT, rt, ret);                                        \
        FK_STRUCT_FIELD(pd1, pt1, a);                                         \
        FK_STRUCT_FIELD(pd2, pt2, b);                                         \
    FK_DECLARE_STRUCT_END(f)
// ------
#define FK_DECLARE_STRUCT_3(rt, f, pd1, pt1, pd2, pt2, pd3, pt3)              \
    FK_DECLARE_STRUCT_START(f);                                               \
        FK_STRUCT_FIELD(OUT, rt, ret);                                        \
        FK_STRUCT_FIELD(pd1, pt1, a);                                         \
        FK_STRUCT_FIELD(pd2, pt2, b);                                         \
        FK_STRUCT_FIELD(pd3, pt3, c);                                         \
    FK_DECLARE_STRUCT_END(f)
// ----------------------------------------------------------------------------
// Complete fake function and structure declarations
// ----------------------------------------------------------------------------
#define DECLARE_FUNC_VOID_0(f)                                                \
    FK_DECLARE_STRUCT_VOID_0(f)
// ------
#define DECLARE_FUNC_VOID_1(f, pd1, pt1)                                      \
    FK_DECLARE_STRUCT_VOID_1(f, pd1, pt1)
// ------
#define DECLARE_FUNC_VOID_2(f, pd1, pt1, pd2, pt2)                            \
    FK_DECLARE_STRUCT_VOID_2(f, pd1, pt1, pd2, pt2)
// ------
#define DECLARE_FUNC_VOID_3(f, pd1, pt1, pd2, pt2, pd3, pt3)                  \
    FK_DECLARE_STRUCT_VOID_3(f, pd1, pt1, pd2, pt2, pd3, pt3)
// ----------------------------------------------------------------------------
#define DECLARE_FUNC_0(rt, f)                                                 \
    FK_DECLARE_STRUCT_0(rt, f)
// ------
#define DECLARE_FUNC_1(rt, f, pd1, pt1)                                       \
    FK_DECLARE_STRUCT_1(rt, f, pd1, pt1)
// ------
#define DECLARE_FUNC_2(rt, f, pd1, pt1, pd2, pt2)                             \
    FK_DECLARE_STRUCT_2(rt, f, pd1, pt1, pd2, pt2)
// ------
#define DECLARE_FUNC_3(rt, f, pd1, pt1, pd2, pt2, pd3, pt3)                   \
    FK_DECLARE_STRUCT_3(rt, f, pd1, pt1, pd2, pt2, pd3, pt3)
// ----------------------------------------------------------------------------
#endif
