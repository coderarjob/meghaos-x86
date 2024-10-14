/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - User Library - System calls
 * -------------------------------------------------------------------------------------------------
 */
#include <types.h>

U32 syscall (U32 fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5)
{
    U32 retval = 0;
    __asm__ volatile("int 0x50"
                     : "=a"(retval) // This is required. Otherwise compiler will not know that eax
                                    // will be changed after this instruction.
                     : "a"(fn), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)
                     :);
    return retval;
}
