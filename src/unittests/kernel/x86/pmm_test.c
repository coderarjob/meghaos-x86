#define YUKTI_TEST_STRIP_PREFIX
#define YUKTI_TEST_IMPLEMENTATION
#include <unittest/yukti.h>
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

void yt_reset(void)
{
    resetBootFake();
}

int main(void)
{
    YT_INIT();
    actual_accessable_ram();
    RETURN_WITH_REPORT();
}
