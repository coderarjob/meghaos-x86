#include <unittest/unittest.h>
#include <mock/kernel/x86/boot.h>
#include <moslimits.h>
#include <pmm.h>
#include <kerror.h>

KernelErrorCodes k_errorNumber;
char *k_errorText[] = {};
void kdebug_printf_ndu (const CHAR *fmt, ...) { }

TEST(PMM, actual_accessable_ram)
{
    kboot_calculateAvailableMemory_fake.ret = 5 * MB;
    size_t available_ram = kpmm_getAddressableByteCount (false);
    EQ_SCALAR (available_ram, 5 * MB);

    available_ram = kpmm_getAddressableByteCount (true);
    EQ_SCALAR (available_ram, 5 * MB);

    kboot_calculateAvailableMemory_fake.ret = 5 * GB;
    available_ram = kpmm_getAddressableByteCount (false);
    EQ_SCALAR (available_ram, MAX_PAB_ADDRESSABLE_BYTE_COUNT);

    available_ram = kpmm_getAddressableByteCount (true);
    EQ_SCALAR (available_ram, MAX_PAB_DMA_ADDRESSABLE_BYTE_COUNT);

    END();
}

void reset()
{
    resetBootFake();

    // Default size of RAM is set to 2 MB.
    kboot_calculateAvailableMemory_fake.ret = 2 * MB;
}

int main()
{
    actual_accessable_ram();
}
