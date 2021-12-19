/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Limits
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 7th September 2021
*/

#ifndef __MOS_LIMITS_H_X86__
#define __MOS_LIMITS_H_X86__
    
    #define MAX_VGA_COLUMNS                 80
    #define MAX_VGA_ROWS                    50
    
    #define MAX_PRINTABLE_STRING_LENGTH     MAX_VGA_COLUMNS * MAX_VGA_ROWS
    
    // Minimum index that can be editted in Kernel.
    #define GDT_MIN_INDEX                   3     

    // Number of GDT entries in memory
    #define GDT_MAX_COUNT                   512   

#endif // __MOS_LIMITS_H_X86__
