#include <types.h>
#include <kstdlib.h>
#include <utils.h>
#include <unittest/unittest.h>

TEST(MEM, memset_one_byte)
{
    U8 dest;
    EQ_SCALAR((PTR)k_memset (&dest, 0x1A, 1), (PTR)&dest);
    EQ_SCALAR(dest, 0x1A);
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

    EQ_SCALAR((PTR)k_memcpy (dest, src, 10), (PTR)dest);

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

    EQ_SCALAR((PTR)k_memcpy (dest, src, ARRAY_LENGTH(dest)), (PTR)&dest);

    for (int i = 0; i < ARRAY_LENGTH(dest); i++)
        EQ_SCALAR(dest[i], src[i]);

    END();
}

TEST(MEM, memset_normal)
{
    U8 dest[10];

    EQ_SCALAR((PTR)k_memset (dest, 0x1A, ARRAY_LENGTH(dest)), (PTR)&dest);

    for (int i = 0; i < ARRAY_LENGTH(dest); i++)
        EQ_SCALAR(dest[i], 0x1A);

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
}
