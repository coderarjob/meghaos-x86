/*
 * Yukti Single Header Mocking and Parameterised unittesting framework
 * Copyright (c) 2022 Arjob Mukherjee (arjobmukherjee@gmail.com)
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ----------------------------------------------------------------------------
 */

#ifndef YUKTI_TEST_H
#define YUKTI_TEST_H

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sys/time.h>
#include <float.h>
#include <math.h>

/*
 * ========================================================================================
 * SECTION 0: SUPPORTING
 * ========================================================================================
 * */

/* ----------------------------------------------------------------------------
 * Intrusive Linked List
 * ----------------------------------------------------------------------------*/
typedef struct ACL_ListNode {
    struct ACL_ListNode* next;
    struct ACL_ListNode* prev;
} ACL_ListNode;

static inline void acl_list_init (ACL_ListNode* node)
{
    node->next = node;
    node->prev = node;
}

static inline bool acl_list_is_empty (ACL_ListNode* head)
{
    return (head->next == head && head->prev == head);
}

static inline void acl_list_add_before (ACL_ListNode* node, ACL_ListNode* item)
{
    item->next       = node;
    item->prev       = node->prev;
    node->prev->next = item;
    node->prev       = item;
}

static inline void acl_list_add_after (ACL_ListNode* node, ACL_ListNode* item)
{
    item->prev       = node;
    item->next       = node->next;
    node->next->prev = item;
    node->next       = item;
}

static inline void acl_list_remove (ACL_ListNode* item)
{
    item->prev->next = item->next;
    item->next->prev = item->prev;
    item->next       = NULL;
    item->prev       = NULL;
}

#define ACL_LIST_ITEM(node, type, member) (type*)((uintptr_t)node - offsetof (type, member))

#define acl_list_for_each(head, node) \
    for ((node) = (head)->next; (node) != (head); (node) = (node)->next)

// ----------------------------------------------------------------------------
// Macros to find out the number of arguments passed to a variadic macro
// ----------------------------------------------------------------------------
#define YT__FIFTEENTH_ELEMENT(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, \
                              ...)                                                              \
    a15
#define YT__COUNT_ARGS(...)                                                                        \
    YT__FIFTEENTH_ELEMENT (dummy __VA_OPT__ (, ) __VA_ARGS__, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, \
                           2, 1, 0)

// ----------------------------------------------------------------------------
// Color template for printing
// ----------------------------------------------------------------------------
#define YT__COL_YELLOW              "\x1b[0m\x1b[1;33m"
#define YT__COL_RED                 "\x1b[0m\x1b[0;31m"
#define YT__COL_GREEN               "\x1b[0m\x1b[0;32m"
#define YT__COL_BOLD_WHITE          "\x1b[0m\x1b[1;97m"
#define YT__COL_WHITE               "\x1b[0m\x1b[0;97m"
#define YT__COL_BLUE_HIGHLIGHT      "\x1b[0m\x1b[1;104m"
#define YT__COL_BLUE_HIGHLIGHT_DARK "\x1b[0m\x1b[1;44m"
#define YT__COL_RESET               "\x1b[0m"

#define YT__PASSED(t) printf ("\n  %sPass%s: %-20s", YT__COL_GREEN, YT__COL_RESET, #t)

#define YT__FAILED(t, fnt, ...)                                                                  \
    do {                                                                                         \
        YT__current_testrecord->failed_exp_count++;                                              \
        printf ("\n  %sFAILED%s  %s : " fnt "%s", YT__COL_RED, YT__COL_RESET, #t, ##__VA_ARGS__, \
                YT__COL_RESET);                                                                  \
    } while (0)

#define YT__PANIC(str)                                               \
    do {                                                             \
        printf ("YT__PANIC! %s.\n", (str) == NULL ? "" : str);       \
        printf ("   at %s:%d in %s.", __func__, __LINE__, __FILE__); \
        exit (1);                                                    \
    } while (0)

// ----------------------------------------------------------------------------
// Declaring, defining and calling functions
// ----------------------------------------------------------------------------
// clang-format off
#define YT__FCALL_WRAP_ARGS_X(w, ...)     YT__FCALL_WRAP_ARGS_0 (w, __VA_ARGS__)
#define YT__FCALL_WRAP_ARGS_0(w, ...)  __VA_OPT__ (YT__FCALL_WRAP_ARGS_1 (w, __VA_ARGS__))
#define YT__FCALL_WRAP_ARGS_1(w, t, ...)  w(_a)__VA_OPT__ (, YT__FCALL_WRAP_ARGS_2 (w, __VA_ARGS__))
#define YT__FCALL_WRAP_ARGS_2(w, t, ...)  w(_b)__VA_OPT__ (, YT__FCALL_WRAP_ARGS_3 (w, __VA_ARGS__))
#define YT__FCALL_WRAP_ARGS_3(w, t, ...)  w(_c)__VA_OPT__ (, YT__FCALL_WRAP_ARGS_4 (w, __VA_ARGS__))
#define YT__FCALL_WRAP_ARGS_4(w, t, ...)  w(_d)__VA_OPT__ (, YT__FCALL_WRAP_ARGS_5 (w, __VA_ARGS__))
#define YT__FCALL_WRAP_ARGS_5(w, t, ...)  w(_e)__VA_OPT__ (, YT__FCALL_WRAP_ARGS_6 (w, __VA_ARGS__))
#define YT__FCALL_WRAP_ARGS_6(w, t, ...)  w(_f)__VA_OPT__ (, YT__FCALL_WRAP_ARGS_7 (w, __VA_ARGS__))
#define YT__FCALL_WRAP_ARGS_7(w, t, ...)  w(_g)__VA_OPT__ (, YT__FCALL_WRAP_ARGS_8 (w, __VA_ARGS__))
#define YT__FCALL_WRAP_ARGS_8(w, t, ...)  w(_h)__VA_OPT__ (, YT__FCALL_WRAP_ARGS_9 (w, __VA_ARGS__))
#define YT__FCALL_WRAP_ARGS_9(w, t, ...)  w(_i)__VA_OPT__ (, YT__FCALL_WRAP_ARGS_10 (w, __VA_ARGS__))
#define YT__FCALL_WRAP_ARGS_10(w, t, ...) w(_j)
// clang-format on

#define YT__FCALL_ARGS_X(...)     YT__FCALL_ARGS_0 (__VA_ARGS__)
#define YT__FCALL_ARGS_0(...)     __VA_OPT__ (YT__FCALL_ARGS_1 (__VA_ARGS__))
#define YT__FCALL_ARGS_1(t, ...)  _a __VA_OPT__ (, YT__FCALL_ARGS_2 (__VA_ARGS__))
#define YT__FCALL_ARGS_2(t, ...)  _b __VA_OPT__ (, YT__FCALL_ARGS_3 (__VA_ARGS__))
#define YT__FCALL_ARGS_3(t, ...)  _c __VA_OPT__ (, YT__FCALL_ARGS_4 (__VA_ARGS__))
#define YT__FCALL_ARGS_4(t, ...)  _d __VA_OPT__ (, YT__FCALL_ARGS_5 (__VA_ARGS__))
#define YT__FCALL_ARGS_5(t, ...)  _e __VA_OPT__ (, YT__FCALL_ARGS_6 (__VA_ARGS__))
#define YT__FCALL_ARGS_6(t, ...)  _f __VA_OPT__ (, YT__FCALL_ARGS_7 (__VA_ARGS__))
#define YT__FCALL_ARGS_7(t, ...)  _g __VA_OPT__ (, YT__FCALL_ARGS_8 (__VA_ARGS__))
#define YT__FCALL_ARGS_8(t, ...)  _h __VA_OPT__ (, YT__FCALL_ARGS_9 (__VA_ARGS__))
#define YT__FCALL_ARGS_9(t, ...)  _i __VA_OPT__ (, YT__FCALL_ARGS_10 (__VA_ARGS__))
#define YT__FCALL_ARGS_10(t, ...) _j

// clang-format off
#define YT__FCALL_ARGS_ARRAY_X(i, ...)     YT__FCALL_ARGS_ARRAY_0 (i, __VA_ARGS__)
#define YT__FCALL_ARGS_ARRAY_0(i, ...)  __VA_OPT__ (YT__FCALL_ARGS_ARRAY_1 (i, __VA_ARGS__))
#define YT__FCALL_ARGS_ARRAY_1(i, t, ...)  _a[i] __VA_OPT__ (, YT__FCALL_ARGS_ARRAY_2 (i, __VA_ARGS__))
#define YT__FCALL_ARGS_ARRAY_2(i, t, ...)  _b[i] __VA_OPT__ (, YT__FCALL_ARGS_ARRAY_3 (i, __VA_ARGS__))
#define YT__FCALL_ARGS_ARRAY_3(i, t, ...)  _c[i] __VA_OPT__ (, YT__FCALL_ARGS_ARRAY_4 (i, __VA_ARGS__))
#define YT__FCALL_ARGS_ARRAY_4(i, t, ...)  _d[i] __VA_OPT__ (, YT__FCALL_ARGS_ARRAY_5 (i, __VA_ARGS__))
#define YT__FCALL_ARGS_ARRAY_5(i, t, ...)  _e[i] __VA_OPT__ (, YT__FCALL_ARGS_ARRAY_6 (i, __VA_ARGS__))
#define YT__FCALL_ARGS_ARRAY_6(i, t, ...)  _f[i] __VA_OPT__ (, YT__FCALL_ARGS_ARRAY_7 (i, __VA_ARGS__))
#define YT__FCALL_ARGS_ARRAY_7(i, t, ...)  _g[i] __VA_OPT__ (, YT__FCALL_ARGS_ARRAY_8 (i, __VA_ARGS__))
#define YT__FCALL_ARGS_ARRAY_8(i, t, ...)  _h[i] __VA_OPT__ (, YT__FCALL_ARGS_ARRAY_9 (i, __VA_ARGS__))
#define YT__FCALL_ARGS_ARRAY_9(i, t, ...)  _i[i] __VA_OPT__ (, YT__FCALL_ARGS_ARRAY_10 (i, __VA_ARGS__))
#define YT__FCALL_ARGS_ARRAY_10(i, t, ...) _j[i]
// clang-format on

#define YT__FUNC_PARAMS_X(...)     YT__FUNC_PARAMS_0 (__VA_ARGS__)
#define YT__FUNC_PARAMS_0(...)     __VA_OPT__ (YT__FUNC_PARAMS_1 (__VA_ARGS__))
#define YT__FUNC_PARAMS_1(t, ...)  t _a __VA_OPT__ (, YT__FUNC_PARAMS_2 (__VA_ARGS__))
#define YT__FUNC_PARAMS_2(t, ...)  t _b __VA_OPT__ (, YT__FUNC_PARAMS_3 (__VA_ARGS__))
#define YT__FUNC_PARAMS_3(t, ...)  t _c __VA_OPT__ (, YT__FUNC_PARAMS_4 (__VA_ARGS__))
#define YT__FUNC_PARAMS_4(t, ...)  t _d __VA_OPT__ (, YT__FUNC_PARAMS_5 (__VA_ARGS__))
#define YT__FUNC_PARAMS_5(t, ...)  t _e __VA_OPT__ (, YT__FUNC_PARAMS_6 (__VA_ARGS__))
#define YT__FUNC_PARAMS_6(t, ...)  t _f __VA_OPT__ (, YT__FUNC_PARAMS_7 (__VA_ARGS__))
#define YT__FUNC_PARAMS_7(t, ...)  t _g __VA_OPT__ (, YT__FUNC_PARAMS_8 (__VA_ARGS__))
#define YT__FUNC_PARAMS_8(t, ...)  t _h __VA_OPT__ (, YT__FUNC_PARAMS_9 (__VA_ARGS__))
#define YT__FUNC_PARAMS_9(t, ...)  t _i __VA_OPT__ (, YT__FUNC_PARAMS_10 (__VA_ARGS__))
#define YT__FUNC_PARAMS_10(t, ...) t _j

// clang-format off
#ifdef __cplusplus
#define YT__FUNC_PARAMS_ARRAY_X(...)     YT__FUNC_PARAMS_ARRAY_0 (__VA_ARGS__)
#define YT__FUNC_PARAMS_ARRAY_0(...)     __VA_OPT__ (YT__FUNC_PARAMS_ARRAY_1 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_1(t, ...)  const t _a[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_2 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_2(t, ...)  const t _b[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_3 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_3(t, ...)  const t _c[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_4 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_4(t, ...)  const t _d[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_5 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_5(t, ...)  const t _e[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_6 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_6(t, ...)  const t _f[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_7 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_7(t, ...)  const t _g[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_8 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_8(t, ...)  const t _h[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_9 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_9(t, ...)  const t _i[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_10 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_10(t, ...) const t _j[]
#else
#define YT__FUNC_PARAMS_ARRAY_X(...)     YT__FUNC_PARAMS_ARRAY_0 (__VA_ARGS__)
#define YT__FUNC_PARAMS_ARRAY_0(...)     __VA_OPT__ (YT__FUNC_PARAMS_ARRAY_1 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_1(t, ...)  t _a[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_2 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_2(t, ...)  t _b[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_3 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_3(t, ...)  t _c[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_4 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_4(t, ...)  t _d[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_5 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_5(t, ...)  t _e[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_6 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_6(t, ...)  t _f[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_7 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_7(t, ...)  t _g[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_8 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_8(t, ...)  t _h[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_9 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_9(t, ...)  t _i[] __VA_OPT__ (, YT__FUNC_PARAMS_ARRAY_10 (__VA_ARGS__))
#define YT__FUNC_PARAMS_ARRAY_10(t, ...) t _j[]
#endif /* __cplusplus */
// clang-format on

/*
 * ========================================================================================
 * SECTION 1: FOR CREATING MOCK FUNCTION DECLARATION & DEFINITIONS
 * ========================================================================================
 * 1.1: RECORD CALL MACROS
 * ========================================================================================
 * */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#ifndef YUKTI_TEST_NO_MUST_CALL

void YT__add_callrecord (ACL_ListNode* head, int sourceLineNumber, const char* const sourceFileName,
                         int n, const char* const fn, ...);

extern ACL_ListNode YT__actualCallListHead;

typedef struct YT__Arg {
    bool isOpt;
    uintptr_t val; // A type large enough to hold both integers & addresses
} YT__Arg;

    #define YT__RECORD_CALL_X(...) YT__FCALL_WRAP_ARGS_X (YT_V, ##__VA_ARGS__)

    #define YT_V(v)                               \
        (YT__Arg)                                 \
        {                                         \
            .isOpt = false, .val = (uintptr_t)(v) \
        }
    #define _ (YT__Arg){ .isOpt = true, .val = 0 }

    #define YT__RECORD_CALL(n, f, ...)                                              \
        do {                                                                        \
            YT__add_callrecord (&YT__actualCallListHead, __LINE__, __FILE__,        \
                                YT__COUNT_ARGS (__VA_ARGS__),                       \
                                #f __VA_OPT__ (, YT__RECORD_CALL_X (__VA_ARGS__))); \
        } while (0)
#else
    #define YT__RECORD_CALL(...) (void)0
#endif /* YUKTI_TEST_NO_MUST_CALL */

/*
 * ========================================================================================
 * SECTION 1: FOR CREATING MOCK FUNCTION DECLARATION & DEFINITIONS
 * ========================================================================================
 * 1.2: MOCKS TO CREATE FAKE/MOCK FUNCTION DEFINITIONS & DECLARATIONS
 * ========================================================================================
 * */
void yt_reset(); // MUST BE DEFINED BY THE USER OF THIS HEADER FILE.

// ----------------------------------------------------------------------------
// Common Helper macros used by both declaration and definition macros.
// ----------------------------------------------------------------------------
#define YT__STRUCT_TAG(f)     f##_fake_tag
#define YT__STRUCT_VAR(f)     f##_fake
#define YT__STRUCT_HANDLER(f) f##_fake_handler

// ---------------------[ FAKE FUNCTION DEFINITION ]-----------------------------
#define YT_DEFINE_FUNC_VOID(f, ...) \
    YT__DEFINE_FUNC_STRUCT (f);     \
    YT__DEFINE_FUNC_BODY_VOID (YT__COUNT_ARGS (__VA_ARGS__), f, __VA_ARGS__)

// ----
#define YT_DEFINE_FUNC(rt, f, ...) \
    YT__DEFINE_FUNC_STRUCT (f);    \
    YT__DEFINE_FUNC_BODY (YT__COUNT_ARGS (__VA_ARGS__), rt, f, __VA_ARGS__)

// ----

// clang-format off
#ifdef __cplusplus
    #define YT__ZEROED { }
#else
    #define YT__ZEROED { 0 }
#endif /* __cplusplus */
// clang-format on

#define YT__DEFINE_FUNC_STRUCT(f) YT__STRUCT_TAG (f) YT__STRUCT_VAR (f) = YT__ZEROED;

#define YT__DEFINE_FUNC_BODY_VOID(n, f, ...) \
    void f (YT__FUNC_PARAMS_X (__VA_ARGS__)) \
    {                                        \
        YT__RECORD_CALL (n, f, __VA_ARGS__); \
        YT__STRUCT_VAR (f).invokeCount++;    \
        YT__RETURN_VOID (f, __VA_ARGS__);    \
    }

#define YT__DEFINE_FUNC_BODY(n, rt, f, ...)  \
    rt f (YT__FUNC_PARAMS_X (__VA_ARGS__))   \
    {                                        \
        YT__RECORD_CALL (n, f, __VA_ARGS__); \
        YT__STRUCT_VAR (f).invokeCount++;    \
        YT__RETURN (f, __VA_ARGS__);         \
    }

#define YT__RETURN_VOID(f, ...)                                      \
    if (YT__STRUCT_VAR (f).handler) {                                \
        YT__STRUCT_VAR (f).handler (YT__FCALL_ARGS_X (__VA_ARGS__)); \
    }

#define YT__RETURN(f, ...)                                                   \
    return (YT__STRUCT_VAR (f).handler)                                      \
               ? YT__STRUCT_VAR (f).handler (YT__FCALL_ARGS_X (__VA_ARGS__)) \
               : YT__STRUCT_VAR (f).ret

// -----------------------[ FAKE FUNCTION DECLARATION ]------------------------
#define YT_DECLARE_FUNC_VOID(f, ...)                \
    typedef void (*f##_fake_handler) (__VA_ARGS__); \
    YT__DECLARE_STRUCT_VOID (f)

// ----
#define YT_DECLARE_FUNC(rt, f, ...)               \
    typedef rt (*f##_fake_handler) (__VA_ARGS__); \
    YT__DECLARE_STRUCT (rt, f)

// ----
#define YT__DECLARE_STRUCT_VOID(f)                      \
    YT__DECLARE_STRUCT_START (f)                        \
    YT__STRUCT_FIELD (void*, resources);                \
    YT__STRUCT_FIELD (unsigned int, invokeCount);       \
    YT__STRUCT_FIELD (YT__STRUCT_HANDLER (f), handler); \
    YT__DECLARE_STRUCT_END (f)

#define YT__DECLARE_STRUCT(rt, f)                       \
    YT__DECLARE_STRUCT_START (f)                        \
    YT__STRUCT_FIELD (void*, resources);                \
    YT__STRUCT_FIELD (unsigned int, invokeCount);       \
    YT__STRUCT_FIELD (rt, ret);                         \
    YT__STRUCT_FIELD (YT__STRUCT_HANDLER (f), handler); \
    YT__DECLARE_STRUCT_END (f)

#define YT__DECLARE_STRUCT_START(f) typedef struct YT__STRUCT_TAG (f) {
#define YT__STRUCT_FIELD(pt, p)     pt p
#define YT__DECLARE_STRUCT_END(f) \
    }                             \
    YT__STRUCT_TAG (f);           \
    extern YT__STRUCT_TAG (f) YT__STRUCT_VAR (f)

// -----------------------[ RESET MOCK ]------------------------
#define YT_RESET_MOCK(f) memset (&YT__STRUCT_VAR (f), 0, sizeof (YT__STRUCT_VAR (f)))

/*
 * ========================================================================================
 * SECTION 2: FOR USAGE IN TEST IMPLEMENTATION TO VALIDATE TEST EXPECTATIONS & REPORTING
 * ========================================================================================
 * 2.1: FOR REPORTING LIST OF FAILED TESTS IN THE END.
 * ========================================================================================
 * */
#ifdef YUKTI_TEST_IMPLEMENTATION

    #define YT__MAX_TEST_FUNCTION_NAME_LENGTH 250
    #define YT__EXIT_FAILURE                  1
    #define YT__EXIT_SUCCESS                  0

typedef struct YT__TestRecord {
    char test_function_name[YT__MAX_TEST_FUNCTION_NAME_LENGTH];
    struct timeval start_time; /* Time when the test started.*/
    uint32_t total_exp_count;
    uint32_t failed_exp_count;
    size_t parameterised_test_number; /* Parameterised Test number (starts from 1) */
    size_t parameterised_test_count;  /* Total number of tests which the same name. */
    ACL_ListNode failedTestListNode;
} YT__TestRecord;

static ACL_ListNode YT__failedTestsListHead;
static YT__TestRecord* YT__current_testrecord = NULL;
static uint32_t YT__total_test_count          = 0;
static uint32_t YT__failed_test_count         = 0;

static YT__TestRecord* YT__create_testRecord (const char* testname, size_t test_count,
                                              size_t test_number)
{
    assert (testname != NULL);

    YT__TestRecord* newrec = NULL;
    if (!(newrec = (YT__TestRecord*)calloc (1, sizeof (YT__TestRecord)))) {
        perror ("malloc");
        YT__PANIC (NULL);
    }

    newrec->total_exp_count           = 0;
    newrec->failed_exp_count          = 0;
    newrec->parameterised_test_number = test_number;
    newrec->parameterised_test_count  = test_count;
    strncpy (newrec->test_function_name, testname, YT__MAX_TEST_FUNCTION_NAME_LENGTH - 1);
    acl_list_init (&newrec->failedTestListNode);

    return newrec;
}

static void YT__free_testRecord (YT__TestRecord* trecord)
{
    // Expectation: Input pointers are not NULL. They are not user facing!
    assert (trecord != NULL);
    free (trecord);
}

    #define YT_INIT()                                 \
        do {                                          \
            acl_list_init (&YT__failedTestsListHead); \
            YT__total_test_count = 0;                 \
        } while (0)

    #define YT_RETURN_WITH_REPORT()                                                    \
        do {                                                                           \
            if (YT__failed_test_count == 0) {                                          \
                printf ("\n%sAll tests passed [0 of %d failed]%s\n", YT__COL_GREEN,    \
                        YT__total_test_count, YT__COL_RESET);                          \
            } else {                                                                   \
                printf ("\n%sNot all tests passed [%d of %d failed]%s", YT__COL_RED,   \
                        YT__failed_test_count, YT__total_test_count, YT__COL_RESET);   \
                ACL_ListNode* node;                                                    \
                acl_list_for_each (&YT__failedTestsListHead, node)                     \
                {                                                                      \
                    YT__TestRecord* test = ACL_LIST_ITEM (node, YT__TestRecord,        \
                                                          failedTestListNode);         \
                    printf ("\n    %s* '%s [%zu/%zu]' test failed%s", YT__COL_RED,     \
                            test->test_function_name, test->parameterised_test_number, \
                            test->parameterised_test_count, YT__COL_RESET);            \
                }                                                                      \
            }                                                                          \
            printf ("\n");                                                             \
            return YT__failed_test_count == 0 ? YT__EXIT_SUCCESS : YT__EXIT_FAILURE;   \
        } while (0)

    /*
     * ========================================================================================
     * SECTION 2: FOR USAGE IN TEST IMPLEMENTATION TO VALIDATE TEST EXPECTATIONS & REPORTING
     * ========================================================================================
     * 2.2: FUNCTION & MACROS TO TEST EXPECTATIONS ON FUNCTION CALLS
     * ========================================================================================
     * */
    #ifdef YUKTI_TEST_NO_MUST_CALL
        // Compilation will fail since the these macros will expand to invalid C code.
        #define YT__ERROR_MESSAGE Invalid when YUKTI_TEST_NO_MUST_CALL is defined

        #define YT_IN_SEQUENCE(n)                         YT__ERROR_MESSAGE
        #define YT_MUST_CALL_IN_ORDER(...)                YT__ERROR_MESSAGE
        #define YT_MUST_CALL_IN_ORDER_ATLEAST_TIMES(...)  YT__ERROR_MESSAGE
        #define YT_MUST_CALL_ANY_ORDER(...)               YT__ERROR_MESSAGE
        #define YT_MUST_CALL_ANY_ORDER_ATLEAST_TIMES(...) YT__ERROR_MESSAGE

        #define YT__validate_expectations() (void)0
        #define YT__teardown()              (void)0
        #define YT__ec_init()               (void)0
    #else
        #define YT_IN_SEQUENCE(n) for (int i = 0; i < (n); i++)

        #define YT_MUST_NEVER_CALL(f, ...)                                                  \
            do {                                                                            \
                YT__current_testrecord->total_exp_count++;                                  \
                YT__add_callrecord (&YT__neverCallExceptationsListHead, __LINE__, __FILE__, \
                                    YT__COUNT_ARGS (__VA_ARGS__) / 2, #f, ##__VA_ARGS__);   \
            } while (0)

        #define YT_MUST_CALL_IN_ORDER(f, ...)                                             \
            do {                                                                          \
                YT__current_testrecord->total_exp_count++;                                \
                YT__add_callrecord (&YT__orderedExceptationListHead, __LINE__, __FILE__,  \
                                    YT__COUNT_ARGS (__VA_ARGS__) / 2, #f, ##__VA_ARGS__); \
            } while (0)

        #define YT_MUST_CALL_IN_ORDER_ATLEAST_TIMES(n, f, ...) \
            for (int i = n; i; i--) {                          \
                YT_MUST_CALL_IN_ORDER (f, ##__VA_ARGS__);      \
            }

        #define YT_MUST_CALL_ANY_ORDER(f, ...)                                            \
            do {                                                                          \
                YT__current_testrecord->total_exp_count++;                                \
                YT__add_callrecord (&YT__globalExceptationListHead, __LINE__, __FILE__,   \
                                    YT__COUNT_ARGS (__VA_ARGS__) / 2, #f, ##__VA_ARGS__); \
            } while (0)

        #define YT_MUST_CALL_ANY_ORDER_ATLEAST_TIMES(n, f, ...) \
            for (int i = n; i; i--) {                           \
                YT_MUST_CALL_ANY_ORDER (f, ##__VA_ARGS__);      \
            }

        #define YT__MAX_CALLSTRING_SIZE      250
        #define YT__MAX_SOURCE_FILE_NAME_LEN 250
        #define YT__ARG_OPTIONAL_CHAR        '!'
        #define YT__ARG_SEPARATOR_CHAR       ','

typedef enum YT__CallRecordTypes {
    YT__CALLRECORD_TYPE_ORDERED_EXPECTATION,
    YT__CALLRECORD_TYPE_GLOBAL_EXPECTATION,
    YT__CALLRECORD_TYPE_ACTUALCALL,
    YT__CALLRECORD_TYPE_NEVER_CALL_EXPECTATION,
} YT__CallRecordTypes;

typedef struct YT__CallRecord {
    char callString[YT__MAX_CALLSTRING_SIZE];
    YT__CallRecordTypes type;
    int sourceLineNumber;
    char* sourceFileName;
    ACL_ListNode listNode;
} YT__CallRecord;

static bool YT__match_call_strings (const char* exp, const char* actual);
static void YT__string_append (char* str, size_t size, const char* const fmt, ...);
static void YT__call_record_free (YT__CallRecord* node);
static void YT__free_call_list (ACL_ListNode* head);
        #ifdef YUKTI_TEST_DEBUG
static void YT__create_call_string (ACL_ListNode* head, char* buffer, size_t buffer_size, int n,
                                    const char* const fn, va_list l);
        #else
static void YT__create_call_string (char* buffer, size_t buffer_size, int n, const char* const fn,
                                    va_list l);
        #endif /* YUKTI_TEST_DEBUG */

static void YT__print_unmet_expectations();
static void YT__validate_expectations();
static void YT__ec_init();
static void YT__teardown();

static ACL_ListNode YT__neverCallExceptationsListHead;
static ACL_ListNode YT__orderedExceptationListHead;
static ACL_ListNode YT__globalExceptationListHead;
ACL_ListNode YT__actualCallListHead;

static bool YT__match_call_strings (const char* exp, const char* actual)
{
    // Expectation: Input pointers are not NULL. They are not user facing!
    assert (actual != NULL && exp != NULL);

    for (; *exp & *actual; exp++) {
        // Expectation: Its not possible to have an optional argument in actual calls
        assert (*actual != YT__ARG_OPTIONAL_CHAR);

        if (*exp == YT__ARG_OPTIONAL_CHAR) {
            for (; *actual && *actual != YT__ARG_SEPARATOR_CHAR && *actual != ')'; actual++)
                ;
            continue;
        } else if (*exp != *actual) {
            break;
        }
        actual++;
    }

    return *actual == *exp;
}

static void YT__string_append (char* str, size_t size, const char* const fmt, ...)
{
    // Expectation: Input pointers are not NULL and Buffer size > 0. They are not user facing!
    assert (str != NULL && fmt != NULL && size > 0);

    size_t len = strlen (str);

    // Expectation: Not possible for string in 'str' to be > size. Because of below check!
    assert (len <= size);

    size -= len;
    str += len;

    va_list l;
    va_start (l, fmt);

    int s = 0;
    if ((s = vsnprintf (str, size, fmt, l)) < 0) {
        perror ("snprintf");
        YT__PANIC (NULL);
    }

    if ((size_t)s >= size) {
        YT__PANIC ("Buffer size too small");
    }

    va_end (l);
}

static void YT__free_call_list (ACL_ListNode* head)
{
    // Expectation: Input pointers are not NULL. They are not user facing!
    assert (head != NULL);

    while (!acl_list_is_empty (head)) {
        YT__CallRecord* item = ACL_LIST_ITEM (head->next, YT__CallRecord, listNode);
        YT__call_record_free (item);
    }
}

static void YT__call_record_free (YT__CallRecord* node)
{
    // Expectation: Input pointers are not NULL. They are not user facing!
    assert (node != NULL);

    acl_list_remove (&node->listNode);
    free (node->sourceFileName);
    free (node);
}

        #ifdef YUKTI_TEST_DEBUG
void YT__create_call_string (ACL_ListNode* head, char* buffer, size_t buffer_size, int n,
                             const char* const fn, va_list l)
        #else
void YT__create_call_string (char* buffer, size_t buffer_size, int n, const char* const fn,
                             va_list l)
        #endif /* YUKTI_TEST_DEBUG */
{
    // Expectation: Input pointers are not NULL and Buffer size > 0. They are not user facing!
    assert (buffer != NULL && fn != NULL && buffer_size > 0);

    YT__string_append (buffer, buffer_size, "%s(", fn);

    for (int i = 0; i < n; i++) {
        YT__Arg item   = va_arg (l, YT__Arg);
        char separator = (i == 0) ? ' ' : YT__ARG_SEPARATOR_CHAR;

        if (item.isOpt) {
        #ifdef YUKTI_TEST_DEBUG
            // Expectation: Actual call list must not have optional arguments
            assert (head != &YT__actualCallListHead);
        #endif /* YUKTI_TEST_DEBUG */
            YT__string_append (buffer, buffer_size, "%c%c", separator, YT__ARG_OPTIONAL_CHAR);
        } else {
            YT__string_append (buffer, buffer_size, "%c%d", separator, item.val);
        }
    }
    YT__string_append (buffer, buffer_size, ")");
    va_end (l);
}

void YT__add_callrecord (ACL_ListNode* head, int sourceLineNumber, const char* const sourceFileName,
                         int n, const char* const fn, ...)
{
    YT__CallRecord* newrec = NULL;
    if (!(newrec = (YT__CallRecord*)malloc (sizeof (YT__CallRecord)))) {
        perror ("malloc");
        YT__PANIC (NULL);
    }
    if (!(newrec->sourceFileName = (char*)malloc (sizeof (char) * YT__MAX_SOURCE_FILE_NAME_LEN))) {
        perror ("malloc");
        YT__PANIC (NULL);
    }

    if (head == &YT__orderedExceptationListHead) {
        newrec->type = YT__CALLRECORD_TYPE_ORDERED_EXPECTATION;
    } else if (head == &YT__globalExceptationListHead) {
        newrec->type = YT__CALLRECORD_TYPE_GLOBAL_EXPECTATION;
    } else if (head == &YT__actualCallListHead) {
        newrec->type = YT__CALLRECORD_TYPE_ACTUALCALL;
    } else if (head == &YT__neverCallExceptationsListHead) {
        newrec->type = YT__CALLRECORD_TYPE_NEVER_CALL_EXPECTATION;
    } else {
        YT__PANIC ("Invalid list");
    }

    acl_list_init (&newrec->listNode);
    acl_list_add_before (head, &newrec->listNode);
    newrec->callString[0]    = '\0';
    newrec->sourceLineNumber = sourceLineNumber;
    strncpy (newrec->sourceFileName, sourceFileName, YT__MAX_SOURCE_FILE_NAME_LEN - 1);

    va_list l;
    va_start (l, fn);
        #ifdef YUKTI_TEST_DEBUG
    YT__create_call_string (head, newrec->callString, sizeof (newrec->callString), n, fn, l);
        #else
    YT__create_call_string (newrec->callString, sizeof (newrec->callString), n, fn, l);
        #endif /* YUKTI_TEST_DEBUG */
    va_end (l);
}

void YT__print_unmet_expectations (ACL_ListNode* neverCallExpectationFailedListHead)
{
    ACL_ListNode* node;
    if (!acl_list_is_empty (neverCallExpectationFailedListHead)) {
        acl_list_for_each (neverCallExpectationFailedListHead, node)
        {
            YT__CallRecord* item = ACL_LIST_ITEM (node, YT__CallRecord, listNode);

            // Never call List must contain only call records of never call expectations
            assert (item->type == YT__CALLRECORD_TYPE_NEVER_CALL_EXPECTATION);

            YT__FAILED (Expectation not met, "Called, when should be never called: %s\n  At: %s:%d",
                        item->callString, item->sourceFileName, item->sourceLineNumber);
        }
    }

    if (!acl_list_is_empty (&YT__globalExceptationListHead)) {
        acl_list_for_each (&YT__globalExceptationListHead, node)
        {
            YT__CallRecord* item = ACL_LIST_ITEM (node, YT__CallRecord, listNode);

            // Global List must contain only call records of global/unordered call expectations
            assert (item->type == YT__CALLRECORD_TYPE_GLOBAL_EXPECTATION);

            YT__FAILED (Expectation not met, "Never called: %s\n  At: %s:%d", item->callString,
                        item->sourceFileName, item->sourceLineNumber);
        }
    }
    if (!acl_list_is_empty (&YT__orderedExceptationListHead)) {
        acl_list_for_each (&YT__orderedExceptationListHead, node)
        {
            YT__CallRecord* item = ACL_LIST_ITEM (node, YT__CallRecord, listNode);

            // Ordered List must contain only call records of Ordered call expectations
            assert (item->type == YT__CALLRECORD_TYPE_ORDERED_EXPECTATION);

            YT__FAILED (Expectation not met, "Never called/called out of order: %s\n  At %s:%d",
                        item->callString, item->sourceFileName, item->sourceLineNumber);
        }
    }

        #ifdef YUKTI_TEST_DEBUG
    printf ("\n  Actual order of functions calls was the following:\n");
    acl_list_for_each (&YT__actualCallListHead, node)
    {
        YT__CallRecord* item = ACL_LIST_ITEM (node, YT__CallRecord, listNode);

        // Actual List must contain only call records of actual calls
        assert (item->type == YT__CALLRECORD_TYPE_ACTUALCALL);

        printf ("    * %s\n", item->callString);
    }
        #endif /* YUKTI_TEST_DEBUG */
}

void YT__validate_expectations()
{
    ACL_ListNode* actCallNode;

    ACL_ListNode neverCallExpectationFailedListHead;
    acl_list_init (&neverCallExpectationFailedListHead);

    acl_list_for_each (&YT__actualCallListHead, actCallNode)
    {
        YT__CallRecord* item = ACL_LIST_ITEM (actCallNode, YT__CallRecord, listNode);

        // Actual List must contain only call records of actual calls
        assert (item->type == YT__CALLRECORD_TYPE_ACTUALCALL);

        ACL_ListNode* neverCallNode;
        acl_list_for_each (&YT__neverCallExceptationsListHead, neverCallNode)
        {
            YT__CallRecord* neverExp = ACL_LIST_ITEM (neverCallNode, YT__CallRecord, listNode);

            // Never call List must contain only call records of never call expectations
            assert (neverExp->type == YT__CALLRECORD_TYPE_NEVER_CALL_EXPECTATION);

            if (YT__match_call_strings (neverExp->callString, item->callString)) {
                acl_list_remove (&neverExp->listNode);
                acl_list_add_before (&neverCallExpectationFailedListHead, &neverExp->listNode);
                break;
            }
        }

        ACL_ListNode* globalCallNode;
        bool globalExpectationMet = false;
        acl_list_for_each (&YT__globalExceptationListHead, globalCallNode)
        {
            YT__CallRecord* gloExp = ACL_LIST_ITEM (globalCallNode, YT__CallRecord, listNode);

            // Global List must contain only call records of global/unordered call expectations
            assert (gloExp->type == YT__CALLRECORD_TYPE_GLOBAL_EXPECTATION);

            if (YT__match_call_strings (gloExp->callString, item->callString)) {
                globalExpectationMet = true;
                YT__call_record_free (gloExp);
                break;
            }
        }

        if (!globalExpectationMet && !acl_list_is_empty (&YT__orderedExceptationListHead)) {
            YT__CallRecord* ordExp = ACL_LIST_ITEM (YT__orderedExceptationListHead.next,
                                                    YT__CallRecord, listNode);

            // Ordered List must contain only call records of Ordered call expectations
            assert (ordExp->type == YT__CALLRECORD_TYPE_ORDERED_EXPECTATION);

            if (YT__match_call_strings (ordExp->callString, item->callString)) {
                YT__call_record_free (ordExp);
            }
        }
    }

    bool success = acl_list_is_empty (&YT__orderedExceptationListHead) &&
                   acl_list_is_empty (&YT__globalExceptationListHead) &&
                   acl_list_is_empty (&neverCallExpectationFailedListHead);

    if (!success) {
        YT__print_unmet_expectations (&neverCallExpectationFailedListHead);
        YT__free_call_list (&neverCallExpectationFailedListHead);
    }
}

static void YT__teardown()
{
    YT__free_call_list (&YT__neverCallExceptationsListHead);
    YT__free_call_list (&YT__globalExceptationListHead);
    YT__free_call_list (&YT__orderedExceptationListHead);
    YT__free_call_list (&YT__actualCallListHead);
}

static void YT__ec_init()
{
    acl_list_init (&YT__neverCallExceptationsListHead);
    acl_list_init (&YT__globalExceptationListHead);
    acl_list_init (&YT__orderedExceptationListHead);
    acl_list_init (&YT__actualCallListHead);
}
    #endif     /* YUKTI_TEST_NO_MUST_CALL */
#endif         /* YUKTI_TEST_IMPLEMENTATION */

/*
 * ========================================================================================
 * SECTION 2: FOR USAGE IN TEST IMPLEMENTATION TO VALIDATE TEST EXPECTATIONS & REPORTING
 * ========================================================================================
 * 2.3: FUNCTION & MACROS TO TEST STATE EXPECTATIONS
 * ========================================================================================
 * */
#ifdef YUKTI_TEST_IMPLEMENTATION
static int YT__equal_mem (const void* a, const void* b, unsigned long size, int* i);
static int YT__equal_string (const char* a, const char* b, int* i);

    #ifdef __cplusplus
        #define AUTOTYPE auto
    #else
        #define AUTOTYPE __auto_type
    #endif /* __cplusplus */

/* Performs either relative or absolute comparison on floating point numbers.
 * Relative comparison: Checks if the difference is smaller than some percentage than the of the two
 * largest number.
 * Absolute comparison: Checks if the difference is smaller than an absolute number. The problem is
 * the floating point accuracy changes depending on how big or small the number is, so the epsilon
 * that works for smaller numbers will not work for larger ones.
 */
static bool yt__approxeq (bool is_abs, double a, double b, double epsilon)
{
    double ut_a    = fabs (a);
    double ut_b    = fabs (b);
    double ut_diff = fabs (a - b);

    // NanS are not numbers, so any compare with numbers must fail
    if (isnan (a) || isnan (b))
        return false;

    // Zero comparison does work well for relative checks, so this takes care of that, also handles
    // the trivial case.
    if ((a == b) || (ut_a == 0.0 && ut_b == 0))
        return true;

    // Infinities are treated as valid, but no comparison is done, instead test matches if both are
    // same, otherwise fails if only one is infinity and the other is not
    if (isinf (ut_a) && isinf (ut_b))
        return true;

    if ((!isinf (ut_a) && isinf (ut_b)) || (isinf (ut_a) && !isinf (ut_b)))
        return false;

    if (is_abs) {
        return (ut_diff <= epsilon);
    } else {
        double ut_largest = (ut_a > ut_b) ? ut_a : ut_b;
        return (ut_diff <= ut_largest * epsilon);
    }
}

    #define YT__ASSERT_FAILED(t, fmt, ...) \
        YT__FAILED (t, fmt "\n  At %s:%d", ##__VA_ARGS__, __FILE__, __LINE__)

    #define YT__TEST_DOUBLE_REL(e, a, o, b)                         \
        do {                                                        \
            YT__current_testrecord->total_exp_count++;              \
            if (!(yt__approxeq (false, a, b, e) o true)) {          \
                YT__ASSERT_FAILED (a o b, "[%f !" #o " %f]", a, b); \
            }                                                       \
        } while (0)

    #define YT__TEST_DOUBLE_ABS(e, a, o, b)                         \
        do {                                                        \
            YT__current_testrecord->total_exp_count++;              \
            if (!(yt__approxeq (true, a, b, e) o true)) {           \
                YT__ASSERT_FAILED (a o b, "[%f !" #o " %f]", a, b); \
            }                                                       \
        } while (0)

    #define YT__TEST_SCALAR(a, o, b)                                              \
        do {                                                                      \
            AUTOTYPE ut_a = (a);                                                  \
            AUTOTYPE ut_b = (b);                                                  \
            YT__current_testrecord->total_exp_count++;                            \
            if (!(ut_a o ut_b))                                                   \
                YT__ASSERT_FAILED (a o b, "[%lld !" #o " %lld]", (long long)ut_a, \
                                   (long long)ut_b);                              \
        } while (0)

    #define YT__TEST_MEM(a, o, b, sz)                                                           \
        do {                                                                                    \
            AUTOTYPE ut_a = (a);                                                                \
            AUTOTYPE ut_b = (b);                                                                \
            YT__current_testrecord->total_exp_count++;                                          \
            int i;                                                                              \
            if (!(YT__equal_mem (ut_a, ut_b, sz, &i) o 1))                                      \
                YT__ASSERT_FAILED (a o b, "[Idx: %d, 0x%X !" #o " 0x%X]", i, ut_a[i], ut_b[i]); \
        } while (0)

    #define YT__TEST_STRING(a, o, b)                                                            \
        do {                                                                                    \
            AUTOTYPE ut_a = (a);                                                                \
            AUTOTYPE ut_b = (b);                                                                \
            YT__current_testrecord->total_exp_count++;                                          \
            int i;                                                                              \
            if (!(YT__equal_string (ut_a, ut_b, &i) o 1))                                       \
                YT__ASSERT_FAILED (a o b, "[Idx: %d, '%c' !" #o " '%c']", i, ut_a[i], ut_b[i]); \
        } while (0)

    #define YT_EQ_DOUBLE_REL(a, b, e)  YT__TEST_DOUBLE_REL (e, a, ==, b)
    #define YT_NEQ_DOUBLE_REL(a, b, e) YT__TEST_DOUBLE_REL (e, a, !=, b)
    #define YT_EQ_DOUBLE_ABS(a, b, e)  YT__TEST_DOUBLE_ABS (e, a, ==, b)
    #define YT_NEQ_DOUBLE_ABS(a, b, e) YT__TEST_DOUBLE_ABS (e, a, !=, b)
    #define YT_EQ_SCALAR(a, b)         YT__TEST_SCALAR (a, ==, b)
    #define YT_NEQ_SCALAR(a, b)        YT__TEST_SCALAR (a, !=, b)
    #define YT_GEQ_SCALAR(a, b)        YT__TEST_SCALAR (a, >=, b)
    #define YT_LEQ_SCALAR(a, b)        YT__TEST_SCALAR (a, <=, b)
    #define YT_LES_SCALAR(a, b)        YT__TEST_SCALAR (a, <, b)
    #define YT_GRT_SCALAR(a, b)        YT__TEST_SCALAR (a, >, b)

    #define YT_EQ_MEM(a, b, sz)  YT__TEST_MEM (a, ==, b, sz)
    #define YT_NEQ_MEM(a, b, sz) YT__TEST_MEM (a, !=, b, sz)

    #define YT_EQ_STRING(a, b)  YT__TEST_STRING (a, ==, b)
    #define YT_NEQ_STRING(a, b) YT__TEST_STRING (a, !=, b)

static int YT__equal_string (const char* a, const char* b, int* i)
{
    *i = 0;
    while (*a && *b && *a == *b) {
        a++;
        b++;
        (*i)++;
    }

    return *a == *b;
}

static int YT__equal_mem (const void* a, const void* b, unsigned long size, int* i)
{
    uint8_t* ta = (uint8_t*)a;
    uint8_t* tb = (uint8_t*)b;

    *i = 0;
    while (size-- && *ta++ == *tb++)
        (*i)++;

    return *--ta == *--tb;
}

    /*
     * ========================================================================================
     * SECTION 2: FOR USAGE IN TEST IMPLEMENTATION TO VALIDATE TEST EXPECTATIONS & REPORTING
     * ========================================================================================
     * 2.4: FUNCTION & MACROS TO PARAMETERISED TEST IMPLEMENTATION
     * ========================================================================================
     * */

    #ifdef __cplusplus
        #define YT_ARG(t) (const t[])
    #else
        #define YT_ARG(t) (t[])
    #endif /* __cplusplus */

    #define YT_ARG_SUB_ARRAY(type, ...) &YT_ARG (type) __VA_ARGS__[0]

    #define YT_ARG_0() _a
    #define YT_ARG_1() _b
    #define YT_ARG_2() _c
    #define YT_ARG_3() _d
    #define YT_ARG_4() _e
    #define YT_ARG_5() _f
    #define YT_ARG_6() _g
    #define YT_ARG_7() _h
    #define YT_ARG_8() _i
    #define YT_ARG_9() _j

    #define YT__TEST_IMPL_BODY(tf, fn, count, tn, ...)                              \
        YT__ec_init();                                                              \
        yt_reset();                                                                 \
        YT__total_test_count++;                                                     \
        /* Following assert ensures we are not overriding it. It was taken cared of \
         * in the previous test's YT_END. */                                        \
        assert (YT__current_testrecord == NULL);                                    \
        YT__current_testrecord = YT__create_testRecord (#fn, count, tn);            \
        if (gettimeofday (&YT__current_testrecord->start_time, NULL)) {             \
            perror ("gettimeofday");                                                \
            YT__PANIC (NULL);                                                       \
        }                                                                           \
        do

    #define YT__TESTP_DECLARE_TEST_FUNC(fn, ...) \
        static void YT__##fn##_test (size_t, size_t, __VA_ARGS__)

    #define YT__TESTP_DEFINE_TEST_FUNC(tf, fn, ...)                                             \
        static void YT__##fn##_test (size_t count, size_t tn, YT__FUNC_PARAMS_X (__VA_ARGS__))  \
        {                                                                                       \
            printf ("%s %s [%lu/%lu] %s:%s %s", YT__COL_BLUE_HIGHLIGHT_DARK, YT__COL_WHITE, tn, \
                    count, #tf, #fn, YT__COL_RESET);                                            \
            YT__TEST_IMPL_BODY (tf, fn, count, tn)

    #define YT__TESTP_DEFINE_TEST_WRAPPER_FUNC(tf, fn, ...)                                        \
        static void fn (size_t count, YT__FUNC_PARAMS_ARRAY_X (__VA_ARGS__))                       \
        {                                                                                          \
            printf ("%s %s %s:%s %s [%lu tests] %s\n", YT__COL_BLUE_HIGHLIGHT, YT__COL_BOLD_WHITE, \
                    #tf, #fn, YT__COL_YELLOW, count, YT__COL_RESET);                               \
            for (unsigned i = 0; i < count; i++) {                                                 \
                YT__##fn##_test (count, i + 1, YT__FCALL_ARGS_ARRAY_X (i, __VA_ARGS__));           \
            }                                                                                      \
        }

    #define YT_TESTP(tf, fn, ...)                                \
        YT__TESTP_DECLARE_TEST_FUNC (fn, __VA_ARGS__);           \
        YT__TESTP_DEFINE_TEST_WRAPPER_FUNC (tf, fn, __VA_ARGS__) \
        YT__TESTP_DEFINE_TEST_FUNC (tf, fn, __VA_ARGS__)

    #define YT_TEST(tf, fn)                                                                      \
        static void fn()                                                                         \
        {                                                                                        \
            printf ("%s %s %s:%s %s", YT__COL_BLUE_HIGHLIGHT_DARK, YT__COL_BOLD_WHITE, #tf, #fn, \
                    YT__COL_RESET);                                                              \
            YT__TEST_IMPL_BODY (tf, fn, 1, 1)

    #ifndef YT__TESTING
static double yt__test_elapsed_time_ms()
{
    struct timeval end_time, start_time = YT__current_testrecord->start_time;
    if (gettimeofday (&end_time, NULL)) {
        perror ("gettimeofday");
        YT__PANIC (NULL);
    }
    return (((end_time.tv_sec - start_time.tv_sec) +
             (end_time.tv_usec - start_time.tv_usec) / 1e6) *
            1000.0);
}

        #define YT__PRINT_SUCCESS_MESSAGE()                                              \
            do {                                                                         \
                printf (" %sOK [%1.4f ms]%s", YT__COL_GREEN, yt__test_elapsed_time_ms(), \
                        YT__COL_RESET);                                                  \
            } while (0)

        #define YT__PRINT_FAILURE_MESSAGE()                                                  \
            do {                                                                             \
                printf ("\n  %s%d of %d failed [%1.4f ms]%s", YT__COL_RED,                   \
                        YT__current_testrecord->failed_exp_count,                            \
                        YT__current_testrecord->total_exp_count, yt__test_elapsed_time_ms(), \
                        YT__COL_RESET);                                                      \
            } while (0)
    #else
        #define YT__PRINT_SUCCESS_MESSAGE()                                      \
            do {                                                                 \
                printf ("  %sOK [0 of %d failed]%s", YT__COL_GREEN,              \
                        YT__current_testrecord->total_exp_count, YT__COL_RESET); \
            } while (0)

        #define YT__PRINT_FAILURE_MESSAGE()                                      \
            do {                                                                 \
                printf ("\n  %s%d of %d failed%s", YT__COL_RED,                  \
                        YT__current_testrecord->failed_exp_count,                \
                        YT__current_testrecord->total_exp_count, YT__COL_RESET); \
            } while (0)
    #endif /* YT__TESTING */

    // clang-format off
    #define YT_END()                                                           \
        YT__validate_expectations();                                           \
        YT__teardown();                                                        \
        if (YT__current_testrecord->failed_exp_count != 0) {                   \
            YT__PRINT_FAILURE_MESSAGE();                                       \
            YT__failed_test_count++;                                           \
            /* Add test to failed test list */                                 \
            acl_list_add_before (&YT__failedTestsListHead,                     \
                                 &YT__current_testrecord->failedTestListNode); \
        } else {                                                               \
            YT__PRINT_SUCCESS_MESSAGE();                                       \
            YT__free_testRecord (YT__current_testrecord);                      \
        }                                                                      \
        YT__current_testrecord = NULL;                                         \
        /* following '}' is for closing YT_TEST's do loop */                   \
        } while (0);                                                           \
        printf ("\n");
    // clang-format on

#endif /* YUKTI_TEST_IMPLEMENTATION */

/* When YUKTI_TEST_STRIP_PREFIX is defined all public interfaces (expect yt_reset & YT_INIT) will
 * have another variant without the 'YT_' prefix.
 */
#ifdef YUKTI_TEST_STRIP_PREFIX
    #define TEST                              YT_TEST
    #define TESTP                             YT_TESTP
    #define END                               YT_END
    #define V                                 YT_V
    #define DEFINE_FUNC_VOID                  YT_DEFINE_FUNC_VOID
    #define DEFINE_FUNC                       YT_DEFINE_FUNC
    #define DECLARE_FUNC_VOID                 YT_DECLARE_FUNC_VOID
    #define DECLARE_FUNC                      YT_DECLARE_FUNC
    #define RESET_MOCK                        YT_RESET_MOCK
    #define IN_SEQUENCE                       YT_IN_SEQUENCE
    #define MUST_CALL_IN_ORDER                YT_MUST_CALL_IN_ORDER
    #define MUST_NEVER_CALL                   YT_MUST_NEVER_CALL
    #define MUST_CALL_IN_ORDER_ATLEAST_TIMES  YT_MUST_CALL_IN_ORDER_ATLEAST_TIMES
    #define MUST_CALL_ANY_ORDER               YT_MUST_CALL_ANY_ORDER
    #define MUST_CALL_ANY_ORDER_ATLEAST_TIMES YT_MUST_CALL_ANY_ORDER_ATLEAST_TIMES
    #define EQ_DOUBLE                         YT_EQ_DOUBLE
    #define NEQ_DOUBLE                        YT_NEQ_DOUBLE
    #define EQ_SCALAR                         YT_EQ_SCALAR
    #define NEQ_SCALAR                        YT_NEQ_SCALAR
    #define GEQ_SCALAR                        YT_GEQ_SCALAR
    #define LEQ_SCALAR                        YT_LEQ_SCALAR
    #define LES_SCALAR                        YT_LES_SCALAR
    #define GRT_SCALAR                        YT_GRT_SCALAR
    #define EQ_MEM                            YT_EQ_MEM
    #define NEQ_MEM                           YT_NEQ_MEM
    #define EQ_STRING                         YT_EQ_STRING
    #define NEQ_STRING                        YT_NEQ_STRING
    #define ARG                               YT_ARG
    #define ARG_SUB_ARRAY                     YT_ARG_SUB_ARRAY
    #define ARG_0                             YT_ARG_0
    #define ARG_1                             YT_ARG_1
    #define ARG_2                             YT_ARG_2
    #define ARG_3                             YT_ARG_3
    #define ARG_4                             YT_ARG_4
    #define ARG_5                             YT_ARG_5
    #define ARG_6                             YT_ARG_6
    #define ARG_7                             YT_ARG_7
    #define ARG_8                             YT_ARG_8
    #define ARG_9                             YT_ARG_9
    #define RETURN_WITH_REPORT                YT_RETURN_WITH_REPORT
#endif /* YUKTI_TEST_STRIP_PREFIX */

#pragma GCC diagnostic pop
#endif // YUKTI_TEST_H
