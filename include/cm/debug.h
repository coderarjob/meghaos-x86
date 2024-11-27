/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Mos libc - Library debug
 * -------------------------------------------------------------------------------------------------
 */

#pragma once

#if defined(DEBUG) && defined(PORT_E9_ENABLED)
    typedef enum DebugLogType {
        DEBUG_LOG_TYPE_INFO,
        DEBUG_LOG_TYPE_FUNC,
        DEBUG_LOG_TYPE_ERROR,
        DEBUG_LOG_TYPE_WARN,
    } DebugLogType;

    void debug_log_ndu (DebugLogType type, const char* func, UINT line, char* fmt, ...);

    #define INFO(...)       debug_log_ndu (DEBUG_LOG_TYPE_INFO, __func__, __LINE__, __VA_ARGS__)
    #define ERROR(...)      debug_log_ndu (DEBUG_LOG_TYPE_ERROR, __func__, __LINE__, __VA_ARGS__)
    #define FUNC_ENTRY(...) debug_log_ndu (DEBUG_LOG_TYPE_FUNC, __func__, __LINE__, "" __VA_ARGS__)
    #define WARN(...)       debug_log_ndu (DEBUG_LOG_TYPE_WARN, __func__, __LINE__, __VA_ARGS__)
#else
    #define INFO(...)       (void)0
    #define ERROR(...)      (void)0
    #define FUNC_ENTRY(...) (void)0
    #define WARN(...)       (void)0
#endif // PORT_E9_ENABLED

#if defined(DEBUG)
    /***************************************************************************************************
     * Magic break point used by bochs emulator
     *
     * @return      Nothing
     **************************************************************************************************/
    #define bochs_breakpoint() __asm__ volatile("xchg bx, bx")
#endif // DEBUG

