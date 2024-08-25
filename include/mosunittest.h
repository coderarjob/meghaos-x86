/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 -  Mocking macros for unit testing
 * ---------------------------------------------------------------------------
 */

#pragma once

#if defined(UNITTEST)
#include <stdint.h>
#include <stddef.h>

typedef struct MockedMacro {
    // Add fields here corresponding to the macro you are mocking
    int recursive_pde_index;
    int kernel_pde_index;
    int temporary_pte_index_extern;
    int temporary_pte_index_internal;
    uintptr_t arch_mem_start_kernel_pab;
    uintptr_t arch_mem_start_salloc;
    size_t arch_mem_len_bytes_salloc;
    size_t arch_mem_len_bytes_kmalloc;
} MockedMacro;

// Need to define it when building unittests
extern MockedMacro g_utmm;

    #define MOCK_THIS_MACRO_USING(mock_variable_name) g_utmm.mock_variable_name
    #define SET_MACRO_MOCK(mock_variable_name, value) g_utmm.mock_variable_name = (value)
#endif // UNITTEST
