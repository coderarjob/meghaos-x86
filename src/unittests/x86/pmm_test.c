#include <kernel.h>
#include <unittest/unittest.h>
#include <mock/kernel/mem.h>
#include <mock/kernel/x86/boot.h>

void* CAST_PA_TO_VA (PHYSICAL a);

PHYSICAL g_pab;
KernelErrorCodes k_errorNumber;

static U8 pab[PAB_SIZE_BYTES] = {0xFF};

TEST(PMM, init)
{
    END();
}

void reset()
{
    resetMemFake();
    resetBootFake();
}

/* Fake Defination. At present, meghatest does not support varidac parameters. */
void k_panic_ndu (const CHAR *s,...) { }
void kdebug_printf_ndu (const CHAR *fmt, ...) { }

int main()
{
    init();
}
