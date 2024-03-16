#include <mock/kernel/x86/memmanage.h>
#include <unittest/fake.h>

DEFINE_FUNC(void*, kmalloc_arch_preAllocateMemory);
DEFINE_FUNC(void*, salloc_arch_preAllocateMemory);

void resetX86MemManageFake()
{
    RESET_FAKE(kmalloc_arch_preAllocateMemory);
    RESET_FAKE(salloc_arch_preAllocateMemory);
}
