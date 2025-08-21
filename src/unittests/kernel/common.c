/*
 * ----------------------------------------------------------------------------
 * Common functions to for use in MeghaOS unitests
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <types.h>
#include <stdarg.h>
#include <kernel.h>
#include <mosunittest.h>

#define COL_GRAY      "\x1b[90m"
#define COL_RESET     "\x1b[0m"

/* MOS Unittest Macro mock global structure */
MockedMacro g_utmm = { 0 };

/* Panic handler */
volatile KernelStateInfo g_kstate;
bool panic_invoked;
void unittest_panic_handler(const CHAR *s,...)
{
    panic_invoked = true;

    va_list l;
    va_start (l, s);

    printf ("%s", COL_GRAY);
    vprintf (s, l);
    printf ("%s", COL_RESET);

    va_end(l);
}
