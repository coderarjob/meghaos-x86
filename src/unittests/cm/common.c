/*
 * ----------------------------------------------------------------------------
 * Common functions to for use in MeghaOS LibCM unitests
 * ----------------------------------------------------------------------------
 */

#include <unittest/unittest.h>
#include <stdio.h>
#include <types.h>
#include <stdarg.h>
#include <kernel.h>
#include <mosunittest.h>

/* MOS Unittest Macro mock global structure */
MockedMacro g_utmm = { 0 };

/* Error number */
uint32_t cm_error_num;

/* Panic handler */
bool cm_panic_invoked;
void cm_unittest_panic_handler()
{
    cm_panic_invoked = true;
}
