#define YUKTI_TEST_STRIP_PREFIX
#define YUKTI_TEST_IMPLEMENTATION
#include <unittest/yukti.h>
#include <types.h>

TEST(types.h, size_t_conformance)
{
    /* Required because:
     * 1. kearly_vsnprintf casts difference of two size_t to an INT.*/
    EQ_SCALAR(sizeof(size_t), sizeof(INT));

    /* Required because:
     * 1. According to C99 standard, size_t must be atleast the size of long int. */
    GEQ_SCALAR(sizeof(size_t), sizeof(LONG));
    END();
}

TEST(types.h, type_sizes)
{
    EQ_SCALAR(sizeof(U8),  1U);
    EQ_SCALAR(sizeof(U16), 2U);
    EQ_SCALAR(sizeof(U32), 4U);
    EQ_SCALAR(sizeof(U64), 8U);
    EQ_SCALAR(sizeof(S32), 4U);
    EQ_SCALAR(sizeof(S64), 8U);

    END();
}

void yt_reset(void)
{
}

int main(void)
{
    YT_INIT();

    type_sizes();
    size_t_conformance();

    RETURN_WITH_REPORT();
}
