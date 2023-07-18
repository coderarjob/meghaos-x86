/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Utils functions
*
* Common functions, which are used not specific to any module but are used throughout the Kernel.
* --------------------------------------------------------------------------------------------------
*/
#include <types.h>

/** Power of two */
UINT power_of_two(UINT e)
{
    UINT result = 1;
    for (; e > 0; e--)
        result = result << 1;
    return result;
}
