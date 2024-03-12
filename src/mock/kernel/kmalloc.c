#include <mock/kernel/kmalloc.h>
#include <unittest/fake.h>

DEFINE_FUNC(void*, s_PreAllocateMemory);

void resetKmallocFake()
{
    RESET_FAKE(s_PreAllocateMemory);
}
