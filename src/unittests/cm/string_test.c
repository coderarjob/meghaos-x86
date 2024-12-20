#include <cm/types.h>
#include <cm/cm.h>
#include <unittest/unittest.h>
#include <mock/kernel/paging.h>

extern bool panic_invoked;

TEST (MEM, memset_one_byte)
{
    U8 dest;
    EQ_SCALAR ((PTR)cm_memset (&dest, 0x1A, 1), (PTR)&dest);
    EQ_SCALAR (dest, 0x1A);
    END();
}

TEST (STRING, string_length)
{
    EQ_SCALAR (cm_strlen (""), 0);
    EQ_SCALAR (cm_strlen ("ABCD"), 4);
    EQ_SCALAR (cm_strlen ("A\n\tB"), 4);
    END();
}

TEST (STRING, string_copy)
{
    char destination[10];
    char* source = NULL;

    // Copy empty string
    source = "";
    EQ_SCALAR ((PTR)cm_strncpy (destination, source, ARRAY_LENGTH (destination)), (PTR)destination);
    EQ_STRING (source, destination);

    // Copy < size of destination
    source = "ABC";
    EQ_SCALAR ((PTR)cm_strncpy (destination, source, ARRAY_LENGTH (destination)), (PTR)destination);
    EQ_STRING (source, destination);

    //// Copy = size of destination
    source = "123456789";
    EQ_SCALAR ((PTR)cm_strncpy (destination, source, ARRAY_LENGTH (destination)), (PTR)destination);
    EQ_STRING (source, destination);

    //// Copy > size of destination
    // source = "123456789ABCD";
    EQ_SCALAR ((PTR)cm_strncpy (destination, source, ARRAY_LENGTH (destination)), (PTR)destination);
    EQ_STRING ("123456789", destination); // Not more than size of destination is copied.
    END();
}

TEST (MEM, memcpy_overlap)
{
    // Here is the memory layout for this test;
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14   < byte indices]
    // |-----------------|   <src]
    // 1 2 3 4 5 6 7 8 9 10  <src contents before copy]
    //           |----------------------| <dest]
    //           1 2 3 4 5 6  7  8  9  10  <dest contents after copy]
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14   < byte indices]

    U8 memory[15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    CHAR* src     = &memory[0];
    CHAR* dest    = &memory[5];

    EQ_SCALAR ((PTR)cm_memcpy (dest, src, 10), (PTR)dest);

    for (int i = 0; i < 10; i++)
        EQ_SCALAR (dest[i], i + 1); // Dest[0 to 9] has 1 to 10

    for (int i = 0; i < 5; i++) {
        EQ_SCALAR (src[i], i + 1);     // Src[0 to 4]  has 1 to 5
        EQ_SCALAR (src[i + 5], i + 1); // Src[5 to 9]  has 1 to 5
    }

    END();
}

TEST (MEM, memcpy_normal)
{
    CHAR* src = "123456789ABCDEF0";
    CHAR dest[10];

    EQ_SCALAR ((PTR)cm_memcpy (dest, src, ARRAY_LENGTH (dest)), (PTR)&dest);

    for (int i = 0; i < ARRAY_LENGTH (dest); i++)
        EQ_SCALAR (dest[i], src[i]);

    END();
}

TEST (MEM, memset_normal)
{
    U8 dest[10];

    EQ_SCALAR ((PTR)cm_memset (dest, 0x1A, ARRAY_LENGTH (dest)), (PTR)&dest);

    for (int i = 0; i < ARRAY_LENGTH (dest); i++)
        EQ_SCALAR (dest[i], 0x1A);

    END();
}

void reset()
{
}

int main()
{
    memset_one_byte();
    memset_normal();
    memcpy_normal();
    memcpy_overlap();
    string_length();
    string_copy();
}
