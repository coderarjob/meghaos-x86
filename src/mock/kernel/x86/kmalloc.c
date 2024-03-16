#include <mock/kernel/x86/kmalloc.h>
#include <unittest/fake.h>

DEFINE_FUNC(void*, kmalloc_arch_preAllocateMemory);

void resetKmallocFake()
{
    RESET_FAKE(kmalloc_arch_preAllocateMemory);
}
