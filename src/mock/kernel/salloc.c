#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <mock/kernel/salloc.h>

DEFINE_FUNC(void*, ksalloc,UINT);
DEFINE_FUNC(void*, kscalloc,UINT);
void reset_sallocFake(void)
{
    RESET_MOCK(ksalloc);
    RESET_MOCK(kscalloc);
}
