#include <kernel.h>
#include <unittest/unittest.h>
#include <mock/kernel/mem.h>
#include <mock/kernel/x86/boot.h>

static U8 pab[PAB_SIZE_BYTES];
KernelErrorCodes k_errorNumber;
PHYSICAL g_pab = PHYSICAL (pab);

/* Fake Defination. At present, meghatest does not support varidac parameters. */
void k_panic_ndu (const CHAR *s,...) { }
void kdebug_printf_ndu (const CHAR *fmt, ...) { }

TEST(PMM, alloc_fixed)
{
    // Will just set pointer `s_pab` to point to `pab` buffer defined here. No allocation or
    // deallocation will take place as memory map count and files count are not set and will be at
    // default zero.
    kpmm_init();

    // Allocating 1 byte means nothing to PMM. It should allocate 1 full page.
    USYSINT bytesCount = 1;
    PHYSICAL startAddress = PHYSICAL (0);

    bool success = kpmm_alloc (NULL, &bytesCount, PMM_FIXED, startAddress);

    EQ_SCALAR (bytesCount, CONFIG_PAGE_FRAME_SIZE_BYTES);
    EQ_SCALAR (pab[0], 0x1);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (success, true);
    END();
}

void reset()
{
    resetMemFake();
    resetBootFake();
}


int main()
{
    alloc_fixed();
}
