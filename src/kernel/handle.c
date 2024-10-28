/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Kernel Object Handle management
 *
 * Handles identify objects in the kernel and can safely be passed to applications as well.
 * --------------------------------------------------------------------------------------------------
 */

#include <handle.h>
#include <stdbool.h>
#include <types.h>
#include <kernel.h>
#include <kdebug.h>
#include <config.h>
#include <memmanage.h>
#include <kerror.h>
#include <kassert.h>

static void** handles = NULL;

static void** getObjectPtr (Handle h)
{
    k_assert (handles != NULL, "Handles not initialized");

    if (h >= CONFIG_HANDLES_ARRAY_ITEM_COUNT) {
        RETURN_ERROR (ERR_INVALID_HANDLE, NULL);
    }

    if (handles[h] == NULL) {
        RETURN_ERROR (ERR_INVALID_HANDLE, NULL);
    }

    return &handles[h];
}

void khandle_init()
{
    FUNC_ENTRY();

    KERNEL_PHASE_VALIDATE (KERNEL_PHASE_STATE_SALLOC_READY);
    if (!(handles = kscalloc (CONFIG_HANDLES_ARRAY_ITEM_COUNT * sizeof (handles)))) {
        FATAL_BUG(); // Should not fail.
    }
}

Handle khandle_createHandle (void* obj)
{
    FUNC_ENTRY ("Object ptr: %px", obj);

    k_assert (handles != NULL, "Handles not initialized");

    if (obj == NULL) {
        RETURN_ERROR (ERR_INVALID_ARGUMENT, INVALID_HANDLE);
    }

    // Find free location
    for (INT i = 0; i < CONFIG_HANDLES_ARRAY_ITEM_COUNT; i++) {
        if (handles[i] == NULL) {
            // Found a free slot in the handles array. Place the object ptr there.
            handles[i] = obj;
            // The index into the Handles array where the object was put is the Handle for it.
            return i;
        }
    }

    // No free slot found.
    RETURN_ERROR (ERR_OUT_OF_MEM, INVALID_HANDLE);
}

bool khandle_freeHandle (Handle h)
{
    FUNC_ENTRY ("Handle: %x", h);

    void** objptr = getObjectPtr (h);
    if (objptr == NULL) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }
    *objptr = NULL;
    return true;
}

void* khandle_getObject (Handle h)
{
    FUNC_ENTRY ("Handle: %x", h);

    void** objptr = getObjectPtr (h);
    if (objptr == NULL) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }
    return *objptr;
}
