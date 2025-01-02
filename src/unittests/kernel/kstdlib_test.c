#include <stdbool.h>
#include <types.h>
#include <utils.h>
#include <unittest/unittest.h>

#ifdef LIBCM
    #include <cm/cm.h>

    #define MEMSET_FN_UNDER_TEST cm_memset
    #define MEMCPY_FN_UNDER_TEST cm_memcpy
    #define STRLEN_FN_UNDER_TEST cm_strlen
    #define STRCPY_FN_UNDER_TEST cm_strncpy
#else
    #include <kstdlib.h>
    #include <mock/kernel/paging.h>

    #define MEMSET_FN_UNDER_TEST k_memset
    #define MEMCPY_FN_UNDER_TEST k_memcpy
    #define STRLEN_FN_UNDER_TEST k_strlen
    #define STRCPY_FN_UNDER_TEST k_strncpy
#endif

#ifndef LIBCM
// The corresponding functions are not available in libcm. So following tests are only for the
// Kernel.
TEST (MEM, memcmp_one_byte)
{
    U8 dest[1] = { 10 };
    U8 src[1]  = { 10 };

    EQ_SCALAR (k_memcmp (dest, src, 1), true);
    END();
}

TEST (MEM, memcmp_empty)
{
    U8 dest[1] = { 10 };
    U8 src[1]  = { 11 };

    EQ_SCALAR (k_memcmp (dest, src, 0), true);
    END();
}

TEST (MEM, memcmp_mismatch_outside_range)
{
    U8 dest[7] = { 1, 2, 3, 4, 5, 1, 2 };
    U8 src[7]  = { 1, 2, 3, 4, 5, 6, 7 };

    EQ_SCALAR (k_memcmp (dest, src, 5), true);
    END();
}

TEST (MEM, memcmp_mismatch_within_range)
{
    U8 dest[7] = { 1, 2, 3, 4, 5, 6, 2 };
    U8 src[7]  = { 1, 2, 3, 4, 5, 6, 7 };

    EQ_SCALAR (k_memcmp (dest, src, 7), false);
    END();
}

TEST (MEMSET_PAT4, memset_pat4_2_bytes_pat)
{
    U32 pattern       = 0x10FF;
    SIZE patBytes     = 2;
    U8 memory[10]     = { 0 };
    U8 memory_exp[10] = { 0xFF, 0x10, 0xFF, 0x10, 0xFF, 0x10, 0xFF, 0x10, 0xFF, 0x10 };

    k_memset_pat4 (memory, pattern, patBytes, ARRAY_LENGTH (memory));
    EQ_MEM (memory, memory_exp, ARRAY_LENGTH (memory));

    END();
}

TEST (MEMSET_PAT4, memset_pat4_3_bytes_pat)
{
    U32 pattern      = 0x1020FF;
    SIZE patBytes    = 3;
    U8 memory[9]     = { 0 };
    U8 memory_exp[9] = { 0xFF, 0x20, 0x10, 0xFF, 0x20, 0x10, 0xFF, 0x20, 0x10 };

    k_memset_pat4 (memory, pattern, patBytes, ARRAY_LENGTH (memory));
    EQ_MEM (memory, memory_exp, ARRAY_LENGTH (memory));

    END();
}

TEST (MEMSET_PAT4, memset_pat4_4_bytes_pat)
{
    U32 pattern      = 0x102030FF;
    SIZE patBytes    = 4;
    U8 memory[8]     = { 0 };
    U8 memory_exp[8] = { 0xFF, 0x30, 0x20, 0x10, 0xFF, 0x30, 0x20, 0x10 };

    k_memset_pat4 (memory, pattern, patBytes, ARRAY_LENGTH (memory));
    EQ_MEM (memory, memory_exp, ARRAY_LENGTH (memory));

    END();
}
#endif

TEST(MEM, memset_one_byte)
{
    U8 dest;
    EQ_SCALAR((PTR)MEMSET_FN_UNDER_TEST (&dest, 0x1A, 1), (PTR)&dest);
    EQ_SCALAR(dest, 0x1A);
    END();
}

TEST(STRING, string_length)
{
    EQ_SCALAR(STRLEN_FN_UNDER_TEST(""), 0);
    EQ_SCALAR(STRLEN_FN_UNDER_TEST("ABCD"), 4);
    EQ_SCALAR(STRLEN_FN_UNDER_TEST("A\n\tB"), 4);
    END();
}

TEST (STRING, string_copy)
{
    char destination[10];
    char* source = NULL;

    // Copy empty string
    source = "";
    EQ_SCALAR ((PTR)STRCPY_FN_UNDER_TEST (destination, source, ARRAY_LENGTH (destination)),
               (PTR)destination);
    EQ_STRING (source, destination);

    // Copy < size of destination
    source = "ABC";
    EQ_SCALAR ((PTR)STRCPY_FN_UNDER_TEST (destination, source, ARRAY_LENGTH (destination)),
               (PTR)destination);
    EQ_STRING (source, destination);

    //// Copy = size of destination
    source = "123456789";
    EQ_SCALAR ((PTR)STRCPY_FN_UNDER_TEST (destination, source, ARRAY_LENGTH (destination)),
               (PTR)destination);
    EQ_STRING (source, destination);

    //// Copy > size of destination
    // source = "123456789ABCD";
    EQ_SCALAR ((PTR)STRCPY_FN_UNDER_TEST (destination, source, ARRAY_LENGTH (destination)),
               (PTR)destination);
    EQ_STRING ("123456789", destination); // Not more than size of destination is copied.
    END();
}

TEST(MEM, memcpy_overlap)
{
    // Here is the memory layout for this test;
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14   < byte indices]
    // |-----------------|   <src]
    // 1 2 3 4 5 6 7 8 9 10  <src contents before copy]
    //           |----------------------| <dest]
    //           1 2 3 4 5 6  7  8  9  10  <dest contents after copy]
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14   < byte indices]

    U8 memory[15] = {1,2,3,4,5,6,7,8,9,10};
    CHAR *src = &memory[0];
    CHAR *dest = &memory[5];

    EQ_SCALAR((PTR)MEMCPY_FN_UNDER_TEST (dest, src, 10), (PTR)dest);

    for (int i = 0; i < 10; i++)
        EQ_SCALAR(dest[i], i + 1);              // Dest[0 to 9] has 1 to 10

    for (int i = 0; i < 5; i++) {
        EQ_SCALAR(src[i], i + 1);              // Src[0 to 4]  has 1 to 5
        EQ_SCALAR(src[i + 5], i + 1);          // Src[5 to 9]  has 1 to 5
    }

    END();
}

TEST(MEM, memcpy_normal)
{
    CHAR *src = "123456789ABCDEF0";
    CHAR dest[10];

    EQ_SCALAR((PTR)MEMCPY_FN_UNDER_TEST (dest, src, ARRAY_LENGTH(dest)), (PTR)&dest);

    for (int i = 0; i < ARRAY_LENGTH(dest); i++)
        EQ_SCALAR(dest[i], src[i]);

    END();
}

TEST(MEM, memset_normal)
{
    U8 dest[10];

    EQ_SCALAR((PTR)MEMSET_FN_UNDER_TEST (dest, 0x1A, ARRAY_LENGTH(dest)), (PTR)&dest);

    for (int i = 0; i < ARRAY_LENGTH(dest); i++)
        EQ_SCALAR(dest[i], 0x1A);

    END();
}

void reset()
{
#ifndef LIBCM
    resetPagingFake();
#endif
}

int main()
{
    memset_one_byte();
    memset_normal();
    memcpy_normal();
    memcpy_overlap();
#ifndef LIBCM
    memset_pat4_2_bytes_pat();
    memset_pat4_3_bytes_pat();
    memset_pat4_4_bytes_pat();
    memcmp_one_byte();
    memcmp_empty();
    memcmp_mismatch_outside_range();
    memcmp_mismatch_within_range();
#endif
    string_length();
    string_copy();
}
