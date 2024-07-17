/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 -  Mocking macros for unit testing
 * ---------------------------------------------------------------------------
 */

#pragma once

#if defined(UNITTEST)
typedef struct MockedMacro {
    // Add fields here corresponding to the macro you are mocking
    int recursive_pde_index;
    int kernel_pde_index;
    int temporary_pte_index_extern;
    int temporary_pte_index_internal;
} MockedMacro;

// Need to define it when building unittests
extern MockedMacro utmm;

    #define MOCK_THIS_MACRO_USING(mock_variable_name) utmm.mock_variable_name
    #define SET_MACRO_MOCK(mock_variable_name, value) utmm.mock_variable_name = (value)
#endif // UNITTEST
