#include <kernel.h>
#include <unittest/unittest.h>
#include <mock/kernel/mem.h>

void* CAST_PA_TO_VA (PHYSICAL_ADDRESS a);

static U8 pab[PAB_SIZE_BYTES] = {0xFF};
PHYSICAL_ADDRESS g_pab;

TEST(PMM, init)
{
    END();
}

int main()
{
    init();
}
