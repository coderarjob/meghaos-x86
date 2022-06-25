/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Configuration
*
* These configurations can be used directly, but are most usually will be used
* to calculate various limits and branch
* ---------------------------------------------------------------------------
*/

#ifndef MOS_CONFIG_X86_H
#define MOS_CONFIG_X86_H
    #define KB 1024
    #define MB 1024 * KB
    #define GB 1024 * MB

    #define CONFIG_PAGE_FRAME_SIZE_BYTES    4 * KB
    #define CONFIG_MIN_RAM_BYTES            2 * MB
    #define CONFIG_VGA_DISPLAY              "80x50"

#endif // MOS_CONFIG_X86_H
