#include <stdint.h>
#include <utils.h>
#include <stddef.h>
#include <unittest/unittest.h>
#include <types.h>

TEST (utils, OffsetOf)
{
    struct TestStruct
    {
        UINT uint_a;
        U16 uint_b;
    } struct_var;

    EQ_SCALAR (offsetof (struct TestStruct, uint_a), offsetOf (struct TestStruct, uint_a));
    EQ_SCALAR (offsetof (struct TestStruct, uint_b), offsetOf (struct TestStruct, uint_b));
    END();
}

TEST (utils, ArrayLength) {
    U32 some_array[10];

    EQ_SCALAR(ARRAY_LENGTH(some_array), 10);

    END();
}

TEST (utils, IsAligned) {
    EQ_SCALAR(IS_ALIGNED(0, 1), true);
    EQ_SCALAR(IS_ALIGNED(2, 1), true);

    EQ_SCALAR(IS_ALIGNED(0, 4096), true);
    EQ_SCALAR(IS_ALIGNED(4096, 4096), true);
    EQ_SCALAR(IS_ALIGNED(4097, 4096), false);

    EQ_SCALAR(IS_ALIGNED(8192, 4096), true);
    END();
}

TEST (utils, AlignUp) {
    EQ_SCALAR(ALIGN_UP(0, 1), 0);
    EQ_SCALAR(ALIGN_UP(1, 4096), 4096);
    EQ_SCALAR(ALIGN_UP(4096, 4096), 4096);
    EQ_SCALAR(ALIGN_UP(4097, 4096), 8192);
    END();
}

TEST (utils, AlignDown) {
    EQ_SCALAR(ALIGN_DOWN(0, 1), 0);
    EQ_SCALAR(ALIGN_DOWN(1, 4096), 0);
    EQ_SCALAR(ALIGN_DOWN(4096, 4096), 4096);
    EQ_SCALAR(ALIGN_DOWN(4097, 4096), 4096);
    END();
}

TEST (utils, Maximum) {
    EQ_SCALAR(MAX(4096, 4096), 4096);
    EQ_SCALAR(MAX(1, 4096), 4096);

    EQ_SCALAR(MAX(INT32_MAX, INT32_MAX - 1), INT32_MAX);
    EQ_SCALAR(MAX(INT32_MAX, INT32_MIN), INT32_MAX);
    END();
}

TEST (utils, Minimum) {
    EQ_SCALAR(MAX(4096, 4096), 4096);
    EQ_SCALAR(MIN(1, 4096), 1);

    EQ_SCALAR(MIN(INT32_MAX, INT32_MAX - 1), INT32_MAX - 1);
    EQ_SCALAR(MIN(INT32_MAX, INT32_MIN), INT32_MIN);
    END();
}

TEST (utils, Bitmask) {
    EQ_SCALAR(BIT_MASK(0, 0), 0x1);
    EQ_SCALAR(BIT_MASK(2, 1), 0x6);

    EQ_SCALAR(BIT_MASK(11, 8), 0xF00);
    EQ_SCALAR(BIT_MASK(12, 8), 0x1F00);

    EQ_SCALAR(BIT_MASK(19, 0), 0xFFFFF);
    EQ_SCALAR(BIT_MASK(31, 0), 0xFFFFFFFF);
    END();
}

TEST (utils, PowerOfTwo) {
    EQ_SCALAR(power_of_two(0), 1);
    EQ_SCALAR(power_of_two(2), 4);
    EQ_SCALAR(power_of_two(3), 8);
    EQ_SCALAR(power_of_two(10), 1024);
    END();
}

TEST (utils, Stringfy) {
    EQ_STRING(STR(1 == 2), "1 == 2");
    EQ_STRING(STR(Arjob), "Arjob");
    END();
}

void reset() {}

int main() {
    Stringfy();
    PowerOfTwo();
    Bitmask();
    Maximum();
    Minimum();
    AlignUp();
    AlignDown();
    IsAligned();
    ArrayLength();
    OffsetOf();
}
