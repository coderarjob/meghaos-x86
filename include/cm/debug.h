/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Mos libc - Library debug
 * -------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#ifndef LIBCM
    #include <cmbuild.h>
#endif

#if defined(DEBUG) && defined(PORT_E9_ENABLED)
typedef enum CM_DebugLogType {
    CM_DEBUG_LOG_TYPE_INFO,
    CM_DEBUG_LOG_TYPE_FUNC,
    CM_DEBUG_LOG_TYPE_ERROR,
    CM_DEBUG_LOG_TYPE_WARN,
} CM_DebugLogType;

void cm_debug_log_ndu (CM_DebugLogType type, const char* func, UINT line, char const* const fmt,
                       ...);

    #define CM_DBG_INFO(...) \
        cm_debug_log_ndu (CM_DEBUG_LOG_TYPE_INFO, __func__, __LINE__, __VA_ARGS__)
    #define CM_DBG_ERROR(...) \
        cm_debug_log_ndu (CM_DEBUG_LOG_TYPE_ERROR, __func__, __LINE__, __VA_ARGS__)
    #define CM_DBG_FUNC_ENTRY(...) \
        cm_debug_log_ndu (CM_DEBUG_LOG_TYPE_FUNC, __func__, __LINE__, "" __VA_ARGS__)
    #define CM_DBG_WARN(...) \
        cm_debug_log_ndu (CM_DEBUG_LOG_TYPE_WARN, __func__, __LINE__, __VA_ARGS__)
#else
    #define CM_DBG_INFO(...)       (void)0
    #define CM_DBG_ERROR(...)      (void)0
    #define CM_DBG_FUNC_ENTRY(...) (void)0
    #define CM_DBG_WARN(...)       (void)0
#endif // PORT_E9_ENABLED

#if defined(DEBUG)
    /***************************************************************************************************
     * Magic break point used by bochs emulator
     *
     * @return      Nothing
     **************************************************************************************************/
    #define cm_debug_bochs_breakpoint() __asm__ volatile("xchg bx, bx")

    #if ARCH == x86
        #define cm_debug_x86_outb(p, v) __asm__ volatile("out dx, al;" : : "a"(v), "d"(p))
        #define cm_debug_x86_inb(p, v)  __asm__ volatile("in al, dx" : "=a"(v) : "d"(p))
    #endif
#endif // DEBUG
