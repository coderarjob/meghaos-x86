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
    #define KB   (1024U)
    #define MB   (1024U * KB)
    #define GB   (1024U * MB)

    #define CONFIG_PAGE_SIZE_BITS           (12U)
    #define CONFIG_PAGE_FRAME_SIZE_BYTES    (4U * KB)
    #define CONFIG_MIN_RAM_BYTES            (2U * MB)
    #define CONFIG_TXMODE_COLUMNS           80 // TODO: Replace MAX_VGA_ROWS, COLUMNS with these.
    #define CONFIG_TXMODE_ROWS              50
    #define CONFIG_MAX_CALL_TRACE_DEPTH     10
    #define CONFIG_PAB_NUMBER_OF_PAGES      1 /* Must be 1. Limitations of current Physical Map */
    #define CONFIG_GXMODE_FONT_WIDTH        8
    #define CONFIG_GXMODE_FONT_HEIGHT       16
    #define CONFIG_GXMODE_XRESOLUTION       800
    #define CONFIG_GXMODE_YRESOLUTION       600
    #define CONFIG_GXMODE_BITSPERPIXEL      32

    #define CONFIG_INTERRUPT_CLOCK_FREQ_HZ  (1000U)

    /** Derived Configs
     * SHOULD NOT BE EDITTED MANUALLY */
    #define CONFIG_PAGE_FRAME_SIZE_BITS         (31U - CONFIG_PAGE_SIZE_BITS)
    #define CONFIG_INTERRUPT_CLOCK_TP_MICROSEC  (1000000U/CONFIG_INTERRUPT_CLOCK_FREQ_HZ)
    #define CONFIG_TICK_PERIOD_MICROSEC         (CONFIG_INTERRUPT_CLOCK_TP_MICROSEC)

#endif // MOS_CONFIG_H_X86
