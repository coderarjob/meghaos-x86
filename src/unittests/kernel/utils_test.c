#include <stdint.h>
#include <utils.h>
#include <stddef.h>
#include <unittest/unittest.h>
#include <types.h>

TEST (utils, clamp)
{
    EQ_SCALAR (CLAMP (0, 0, 0), 0);
    EQ_SCALAR (CLAMP (-10, 1, 2), 1);
    EQ_SCALAR (CLAMP (10, 1, 2), 2);
    END();
}

TEST (utils, bit_set)
{
    EQ_SCALAR (BIT_SET (0x10, 0), 0b00010001);
    EQ_SCALAR (BIT_SET (0x10, 7), 0b10010000);
    EQ_SCALAR (BIT_SET (0x10, 6) | BIT_SET (0x10, 3), 0b01011000);
    EQ_SCALAR (BIT_SET (0x10, 4), 0x10);

    size_t b = (sizeof (unsigned int) * 8) - 1;
    EQ_SCALAR (BIT_SET (0x0, b), (1 << b));

    END();
}

TEST (utils, bit_clear)
{
    EQ_SCALAR (BIT_CLEAR (0x10, 4), 0b00000000);
    EQ_SCALAR (BIT_CLEAR (0x10, 0), 0x10);

    EQ_SCALAR (BIT_CLEAR (0xFF, 7), 0b01111111);
    EQ_SCALAR (BIT_CLEAR (0xFF, 0), 0b11111110);
    EQ_SCALAR (BIT_CLEAR (0xFF, 6) & BIT_CLEAR (0xFF, 3), 0b10110111);

    size_t b = (sizeof (unsigned int) * 8) - 1;
    EQ_SCALAR (BIT_CLEAR (0xFFFFFFFF, b), 0x7FFFFFFF);

    END();
}

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
    EQ_SCALAR(POWER_OF_TWO(0), 1);
    EQ_SCALAR(POWER_OF_TWO(2), 4);
    EQ_SCALAR(POWER_OF_TWO(3), 8);
    EQ_SCALAR(POWER_OF_TWO(10), 1024);
    END();
}

TEST (utils, Stringfy) {
    EQ_STRING(STR(1 == 2), "1 == 2");
    EQ_STRING(STR(Arjob), "Arjob");
    END();
}

TEST (utils, bit_isunset) {
    int flags = (1 << 13) | (1 << 10) | (1 << 2);

    EQ_SCALAR(0U, BIT_ISUNSET(flags, (1 << 13)));
    EQ_SCALAR(0U, BIT_ISUNSET(flags, (1 << 10)));
    EQ_SCALAR(0U, BIT_ISUNSET(flags, (1 << 2)));

    EQ_SCALAR(1U, BIT_ISUNSET(flags, (1 << 12)));
    EQ_SCALAR(1U, BIT_ISUNSET(flags, (1 << 24)));
    EQ_SCALAR(1U, BIT_ISUNSET(flags, (1 << 1)));
    END();
}

TEST (utils, bit_isset) {
    int flags = (1 << 13) | (1 << 10) | (1 << 2);

    EQ_SCALAR(1U, BIT_ISSET(flags, (1 << 13)));
    EQ_SCALAR(1U, BIT_ISSET(flags, (1 << 10)));
    EQ_SCALAR(1U, BIT_ISSET(flags, (1 << 2)));

    EQ_SCALAR(0U, BIT_ISSET(flags, (1 << 24)));
    EQ_SCALAR(0U, BIT_ISSET(flags, (1 << 14)));
    EQ_SCALAR(0U, BIT_ISSET(flags, (1 << 1)));
    END();
}

TEST (utils, bytes_to_pageframes_floor) {
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_FLOOR(0x1), 0x0);
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_FLOOR(4095), 0x0);
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_FLOOR(4096), 0x1);
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_FLOOR(4097), 0x1);
    END();
}

TEST (utils, bytes_to_pageframes_ceiling) {
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_CEILING(0x1), 0x1);
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_CEILING(4095), 0x1);
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_CEILING(4096), 0x1);
    EQ_SCALAR(BYTES_TO_PAGEFRAMES_CEILING(4097), 0x2);

    END();
}

TEST (utils, pageframes_to_bytes) {
    EQ_SCALAR(PAGEFRAMES_TO_BYTES(0x0), 0x0);
    EQ_SCALAR(PAGEFRAMES_TO_BYTES(0x1), CONFIG_PAGE_FRAME_SIZE_BYTES);
    EQ_SCALAR(PAGEFRAMES_TO_BYTES(0x5), 5 * CONFIG_PAGE_FRAME_SIZE_BYTES);

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
    bit_isset();
    bit_isunset();
    pageframes_to_bytes();
    bytes_to_pageframes_ceiling();
    bytes_to_pageframes_floor();
    bit_set();
    bit_clear();
    clamp();
}
