#include <stdint.h>
#define YUKTI_TEST_STRIP_PREFIX
#define YUKTI_TEST_IMPLEMENTATION
#include <unittest/yukti.h>
#include <stdio.h>
#include <string.h>
#include <mock/kernel/salloc.h>
#include <config.h>
#include <handle.h>
#include <kerror.h>
#include <stdlib.h>

#define HANDLES_ARRAY_COUNT 100
#define HANDLES_ARRAY_SIZE_BYTES (sizeof(void*) * HANDLES_ARRAY_COUNT)
static void* ut_handles;
#define FIRST_HANDLE 0
#define LAST_HANDLE HANDLES_ARRAY_COUNT - 1

/*
 * | TEST CASES                                            | TEST FUNCTION                 |
 * |-------------------------------------------------------|-------------------------------|
 * | Handle init failed                                    | init_faliure                  |
 * | Create Handle to NULL object pointer                  | add_null_obj_inadd_failure    |
 * | Create handle when handles array is empty             | add_get_first_success         |
 * | Create handle when handles array is partially full    | add_get_last_success          |
 * | Create handle when handles array is full              | add_oom_failure               |
 * | Get object from Handle (first, last) in handles array | add_get_first_success         |
 * |                                                       | add_get_last_success          |
 * | Remove Handle Success                                 | remove_object_success         |
 * | Invalid handle: Exceeding max value for any handle    | get_invalid_handle_failure    |
 * |                                                       | remove_invalid_handle_failure |
 * | Invalid handle: Handle poiting to NULL object         | get_invalid_handle_failure    |
 * |                                                       | remove_invalid_handle_failure |
 * |-------------------------------------------------------|-------------------------------|
 * */

TEST (handles, init_failure)
{
    // Pre setup:
    // Memory allocation failed
    // ------------------
    kscalloc_fake.ret = NULL;
    // ------------------

    khandle_init();
    EQ_SCALAR (panic_invoked, true);
    END();
}

TEST (handles, add_get_first_success)
{
    // Pre setup:
    // ------------------
    // None
    // ------------------

    int obj1  = 0xAAFFBB00;
    Handle h1 = khandle_createHandle (&obj1);
    EQ_SCALAR (h1, FIRST_HANDLE);

    EQ_SCALAR ((PTR)khandle_getObject (h1), (PTR)&obj1);
    EQ_SCALAR (*(int*)khandle_getObject (h1), obj1);

    END();
}

TEST (handles, add_get_last_success)
{
    // Pre setup:
    // Fill all but the last item of the handles array will non NULL value.
    // ------------------
    memset (ut_handles, 0x1, HANDLES_ARRAY_SIZE_BYTES - sizeof (void*));
    // ------------------

    int obj1  = 0xAAFFBB00;
    Handle h1 = khandle_createHandle (&obj1);
    EQ_SCALAR (h1, LAST_HANDLE);

    EQ_SCALAR ((PTR)khandle_getObject (h1), (PTR)&obj1);
    EQ_SCALAR (*(int*)khandle_getObject (h1), obj1);

    END();
}

TEST (handles, add_null_obj_inadd_failure)
{
    // Pre setup:
    // ------------------
    // None
    // ------------------

    EQ_SCALAR (khandle_createHandle (NULL), INVALID_HANDLE);
    EQ_SCALAR (g_kstate.errorNumber, (UINT)ERR_INVALID_ARGUMENT);
    END();
}

TEST (handles, add_oom_failure)
{
    // Pre setup:
    // Fill the handles array will non NULL value to simulate handles array full scenario.
    // ------------------
    memset (ut_handles, 0x1, HANDLES_ARRAY_SIZE_BYTES);
    // ------------------

    int obj1 = 0xAAFFBB00;
    EQ_SCALAR (khandle_createHandle (&obj1), INVALID_HANDLE);
    EQ_SCALAR (g_kstate.errorNumber, (UINT)ERR_OUT_OF_MEM);
    END();
}

TEST (handles, get_invalid_handle_failure)
{
    // Pre setup:
    // add one object to obtain its handle
    // ------------------
    int obj1  = 0xAAFFBB00;
    Handle h1 = khandle_createHandle (&obj1);
    NEQ_SCALAR (h1, INVALID_HANDLE);
    // ------------------

    // Invalid handle 1: Exceeds the Max count
    EQ_SCALAR ((PTR)khandle_getObject (HANDLES_ARRAY_COUNT), (PTR)NULL);
    EQ_SCALAR (g_kstate.errorNumber, (UINT)ERR_INVALID_HANDLE);

    // Invalid handle 1: Handle that points to NULL object
    EQ_SCALAR ((PTR)khandle_getObject (h1 + 1), (PTR)NULL);
    EQ_SCALAR (g_kstate.errorNumber, (UINT)ERR_INVALID_HANDLE);
    END();
}

TEST (handles, remove_invalid_handle_failure)
{
    // Pre setup:
    // add one object to obtain its handle
    // ------------------
    int obj1  = 0xAAFFBB00;
    Handle h1 = khandle_createHandle (&obj1);
    NEQ_SCALAR (h1, INVALID_HANDLE);
    // ------------------

    // Invalid handle 1: Exceeds the Max count
    EQ_SCALAR ((PTR)khandle_freeHandle (HANDLES_ARRAY_COUNT), (PTR)NULL);
    EQ_SCALAR (g_kstate.errorNumber, (UINT)ERR_INVALID_HANDLE);

    // Invalid handle 1: Handle that points to NULL object
    EQ_SCALAR ((PTR)khandle_freeHandle (h1 + 1), (PTR)NULL);
    EQ_SCALAR (g_kstate.errorNumber, (UINT)ERR_INVALID_HANDLE);
    END();
}

TEST (handles, remove_object_success)
{
    // Pre setup:
    // add three objects to obtain its handle
    // ------------------
    int obj1  = 0xAAFFBB00;
    Handle h1 = khandle_createHandle (&obj1);
    Handle h2 = khandle_createHandle (&obj1);
    Handle h3 = khandle_createHandle (&obj1);
    NEQ_SCALAR (h1, INVALID_HANDLE);
    NEQ_SCALAR (h2, INVALID_HANDLE);
    NEQ_SCALAR (h3, INVALID_HANDLE);
    // ------------------

    // Free the handle for the middle object
    EQ_SCALAR (khandle_freeHandle (h2), true);

    // Adding a object again should give the same handle.
    EQ_SCALAR (khandle_createHandle (&obj1), h2);
    END();
}

void yt_reset(void)
{
    memset (ut_handles, 0x0, HANDLES_ARRAY_SIZE_BYTES);
    reset_sallocFake();
    kscalloc_fake.ret = ut_handles;
    khandle_init();
}

int main(void)
{
    YT_INIT();
    g_utmm.config_handles_array_item_count = HANDLES_ARRAY_COUNT;
    ut_handles = malloc(HANDLES_ARRAY_SIZE_BYTES);

    init_failure();
    add_null_obj_inadd_failure();
    add_oom_failure();
    add_get_first_success();
    add_get_last_success();
    get_invalid_handle_failure();
    remove_object_success();
    remove_invalid_handle_failure();
    RETURN_WITH_REPORT();
}
