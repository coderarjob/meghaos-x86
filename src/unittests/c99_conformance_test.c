#include <kernel.h>
#include <unittest/unittest.h>

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
    EQ_SCALAR(sizeof(U8), 1);
    EQ_SCALAR(sizeof(U16), 2);
    EQ_SCALAR(sizeof(U32), 4);
    EQ_SCALAR(sizeof(U64), 8);

    EQ_SCALAR(sizeof(S32), 4);
    EQ_SCALAR(sizeof(S64), 8);

    END();
}

void reset()
{
}

int main()
{
    type_sizes();
    size_t_conformance();
}
