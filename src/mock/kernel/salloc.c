#include <unittest/fake.h>
#include <mock/kernel/salloc.h>

DEFINE_FUNC(void*, ksalloc,UINT);
DEFINE_FUNC(void*, kscalloc,UINT);
void reset_sallocFake()
{
    RESET_FAKE(ksalloc);
    RESET_FAKE(kscalloc);
}
