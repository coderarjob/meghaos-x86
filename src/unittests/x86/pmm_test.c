#include <kernel.h>
#include <unittest/unittest.h>
#include <mock/kernel/mem.h>
#include <mock/kernel/x86/boot.h>
#include <string.h>

static U8 pab[PAB_SIZE_BYTES];

KernelErrorCodes k_errorNumber;
char *k_errorText[] = {};
PHYSICAL g_pab = PHYSICAL ((USYSINT)pab);

/* Fake Defination. At present, meghatest does not support varidac parameters. */
void* CAST_PA_TO_VA (PHYSICAL a);
void k_panic_ndu (const CHAR *s,...) { }
void kdebug_printf_ndu (const CHAR *fmt, ...) { }

TEST(PMM, alloc_fixed_auto)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 134217728 should throw error.
    USYSINT pagesCount = 3;
    PHYSICAL startAddress;

    INT success = kpmm_alloc (&startAddress, pagesCount, PMM_AUTOMATIC, (USYSINT)0);

    EQ_SCALAR (startAddress.val, 0);
    EQ_SCALAR (pab[0], 0x07);
    EQ_SCALAR (pab[1], 0x00);
    EQ_SCALAR (success, EXIT_SUCCESS);

    success = kpmm_alloc (&startAddress, pagesCount, PMM_AUTOMATIC, (USYSINT)0);

    EQ_SCALAR (startAddress.val, 3 * 4096);
    EQ_SCALAR (pab[0], 0x3F);
    EQ_SCALAR (pab[1], 0x00);
    EQ_SCALAR (success, EXIT_SUCCESS);
    END();
}

TEST(PMM, alloc_fixed_past_last_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 134217728 should throw error.
    USYSINT pagesCount = 1;
    USYSINT startAddress = ALIGN_UP (MAX_ADDRESSABLE_BYTE, CONFIG_PAGE_FRAME_SIZE_BYTES);

    INT success = kpmm_alloc (NULL, pagesCount, PMM_FIXED, startAddress);

    EQ_SCALAR (pab[4094], 0x0);
    EQ_SCALAR (pab[4095], 0x00);
    EQ_SCALAR (success, EXIT_FAILURE);
    EQ_SCALAR (k_errorNumber, ERR_OUTSIDE_ADDRESSABLE_RANGE);
    END();
}

TEST(PMM, alloc_fixed_last_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 134213632 (start of last addressable page) should succeed.
    USYSINT pagesCount = 1;
    USYSINT startAddress = ALIGN_DOWN (MAX_ADDRESSABLE_BYTE, CONFIG_PAGE_FRAME_SIZE_BYTES);

    INT success = kpmm_alloc (NULL, pagesCount, PMM_FIXED, startAddress);

    EQ_SCALAR (pab[4094], 0x0);
    EQ_SCALAR (pab[4095], 0x80);
    EQ_SCALAR (success, EXIT_SUCCESS);
    END();
}

TEST(PMM, alloc_fixed_misaligned_address)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 1. Should throw ERR_WRONG_ALIGNMENT.
    USYSINT pagesCount = 1;
    USYSINT startAddress = 1;

    INT success = kpmm_alloc (NULL, pagesCount, PMM_FIXED, startAddress);

    EQ_SCALAR (pab[0], 0x0);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (success, EXIT_FAILURE);
    EQ_SCALAR (k_errorNumber, ERR_WRONG_ALIGNMENT);
    END();
}

TEST(PMM, alloc_fixed_1_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 0. Should be successful.
    USYSINT pagesCount = 1;
    USYSINT startAddress = 0;

    INT success = kpmm_alloc (NULL, pagesCount, PMM_FIXED, startAddress);

    EQ_SCALAR (pab[0], 0x1);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (success, EXIT_SUCCESS);
    EQ_SCALAR (k_errorNumber, ERR_NONE);
    END();
}

void reset()
{
    resetMemFake();
    resetBootFake();
}


int main()
{
    // Will just set pointer `s_pab` to point to `pab` buffer defined here. No allocation or
    // deallocation will take place as memory map count and files count are not set and will be at
    // default zero.
    kpmm_init();

    alloc_fixed_1_page();
    alloc_fixed_misaligned_address();
    alloc_fixed_last_page();
    alloc_fixed_past_last_page();
    alloc_fixed_auto();
}
