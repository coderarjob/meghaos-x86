#include <unittest/unittest.h>
#include <mock/kernel/x86/boot.h>
#include <moslimits.h>
#include <pmm.h>
#include <kerror.h>

TEST(PMM, actual_accessable_ram)
{
    kboot_calculateInstalledMemory_fake.ret = 5 * MB;
    size_t available_ram = kpmm_arch_getInstalledMemoryByteCount();
    EQ_SCALAR (available_ram, 5 * MB);

    kboot_calculateInstalledMemory_fake.ret = 16 * GB;
    available_ram = kpmm_arch_getInstalledMemoryByteCount();
    EQ_SCALAR (available_ram, 16 * GB);

    END();
}

void reset()
{
    resetBootFake();
}

int main()
{
    actual_accessable_ram();
}
