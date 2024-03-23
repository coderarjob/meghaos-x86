/*
 * ----------------------------------------------------------------------------
 * Common functions to for use in MeghaOS unitests
 * ----------------------------------------------------------------------------
 */

#include <unittest/unittest.h>
#include <stdio.h>
#include <types.h>
#include <stdarg.h>

/* Panic handler */
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
