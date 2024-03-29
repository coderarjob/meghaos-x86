/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Configuration
*
* These configurations can be used directly, but are most usually will be used
* to calculate various limits and branch
* ---------------------------------------------------------------------------
*/

#ifndef MOS_CONFIG_H_X86
#define MOS_CONFIG_H_X86

    #include <buildcheck.h>

    #define Byte (1U)
    #define KB (1024U)
    #define MB (1024U * KB)
    #define GB (1024U * MB)

    #define CONFIG_PAGE_SIZE_BITS           (12U)
    #define CONFIG_PAGE_FRAME_SIZE_BYTES    (4U * KB)
    #define CONFIG_MIN_RAM_BYTES            (2U * MB)
    #define CONFIG_VGA_DISPLAY              "80x50"
    #define CONFIG_MAX_CALL_TRACE_DEPTH     10
    #define CONFIG_PAB_NUMBER_OF_PAGES      1 /* Must be 1. Limitations of current Physical Map */

    /** Derived Configs
     * SHOULD NOT BE EDITTED MANUALLY */
    #define CONFIG_PAGE_FRAME_SIZE_BITS     (31U - CONFIG_PAGE_SIZE_BITS)

#endif // MOS_CONFIG_H_X86
