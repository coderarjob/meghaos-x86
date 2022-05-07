/*
 * ----------------------------------------------------------------------------
 * Functions for equality and negation checks.
 *
 * Author: arjobmukherjee@gmail.com (Arjob Mukherjee)
 * Dated : 26 April 2022
 * ----------------------------------------------------------------------------
 */
#include <unittest/unittest.h>
#include <stdint.h>

typedef uint8_t byte;

int ut_equal_string (const char *a, const char *b, int *i)
{
    *i = 0;
    while (*a && *b && *a == *b) {
        a++;
        b++;
        (*i)++;
    }

    return *a == *b;
}

int ut_equal_mem (const void *a, const void *b, unsigned long size, int *i)
{
    *i = 0;
    while (size-- && *(byte *)a++ == *(byte *)b++)
        (*i)++ ;

    return *(byte *)--a == *(byte *)--b;
}
