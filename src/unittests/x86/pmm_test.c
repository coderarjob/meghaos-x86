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

TEST(PMM, autoalloc_out_of_memory)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating every page. Then second allocation will fail.
    PHYSICAL startAddress;
    INT success;

    success = kpmm_alloc (&startAddress, MAX_ADDRESSABLE_PAGE_COUNT, PMM_AUTOMATIC, (USYSINT)0);
    EQ_SCALAR (startAddress.val, 0);
    EQ_SCALAR (pab[0], 0xFF);
    EQ_SCALAR (pab[4095], 0xFF);
    EQ_SCALAR (success, EXIT_SUCCESS);

    // Allocating 1 more page. Should fail as there are no free pages.
    success = kpmm_alloc (&startAddress, 1, PMM_AUTOMATIC, (USYSINT)0);
    EQ_SCALAR (k_errorNumber, ERR_OUT_OF_MEM);
    EQ_SCALAR (success, EXIT_FAILURE);
    END();
}

TEST(PMM, autoalloc_free_autoalloc)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 3 pages. Address should be allocated automatically. 
    PHYSICAL startAddress;

    INT success = kpmm_alloc (&startAddress, 3, PMM_AUTOMATIC, (USYSINT)0);

    EQ_SCALAR (startAddress.val, 0);
    EQ_SCALAR (pab[0], 0x07);
    EQ_SCALAR (pab[1], 0x00);
    EQ_SCALAR (success, EXIT_SUCCESS);

    // Allocating 3 pages again. Address should be allocated automatically. 
    success = kpmm_alloc (&startAddress, 3, PMM_AUTOMATIC, (USYSINT)0);

    EQ_SCALAR (startAddress.val, 3 * 4096);
    EQ_SCALAR (pab[0], 0x3F);
    EQ_SCALAR (pab[1], 0x00);
    EQ_SCALAR (success, EXIT_SUCCESS);

    // Now free 2 pages.
    success = kpmm_free (startAddress, 2);
    EQ_SCALAR (pab[0], 0x27);
    EQ_SCALAR (pab[1], 0x00);
    EQ_SCALAR (success, EXIT_SUCCESS);

    // Allocating 3 pages again. Address should be allocated automatically. 
    success = kpmm_alloc (&startAddress, 3, PMM_AUTOMATIC, (USYSINT)0);

    EQ_SCALAR (startAddress.val, 6 * 4096);
    EQ_SCALAR (pab[0], 0xE7);
    EQ_SCALAR (pab[1], 0x01);
    EQ_SCALAR (success, EXIT_SUCCESS);

    END();
}

TEST(PMM, alloc_auto)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 3 pages. Address should be allocated automatically. 
    USYSINT pagesCount = 3;
    PHYSICAL startAddress;

    INT success = kpmm_alloc (&startAddress, pagesCount, PMM_AUTOMATIC, (USYSINT)0);

    EQ_SCALAR (startAddress.val, 0);
    EQ_SCALAR (pab[0], 0x07);
    EQ_SCALAR (pab[1], 0x00);
    EQ_SCALAR (success, EXIT_SUCCESS);

    // Allocating 3 pages again. Address should be allocated automatically. 
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

TEST(PMM, alloc_auto_last_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating every but leaving the last page. 
    PHYSICAL startAddress;
    INT success;

    success = kpmm_alloc (&startAddress, MAX_ADDRESSABLE_PAGE_COUNT - 1, PMM_AUTOMATIC, (USYSINT)0);
    EQ_SCALAR (startAddress.val, 0);
    EQ_SCALAR (pab[0], 0xFF);
    EQ_SCALAR (pab[4095], 0x7F);
    EQ_SCALAR (success, EXIT_SUCCESS);

    // Allocating 1 more page.
    success = kpmm_alloc (&startAddress, 1, PMM_AUTOMATIC, (USYSINT)0);
    EQ_SCALAR (pab[0], 0xFF);
    EQ_SCALAR (pab[4095], 0xFF);
    EQ_SCALAR (success, EXIT_SUCCESS);
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

TEST(PMM, alloc_free_fixed_misaligned_address)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 1. Should throw ERR_WRONG_ALIGNMENT.
    USYSINT startAddress = 1;
    INT success;

    success = kpmm_alloc (NULL, 1, PMM_FIXED, startAddress);

    EQ_SCALAR (pab[0], 0x0);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (success, EXIT_FAILURE);
    EQ_SCALAR (k_errorNumber, ERR_WRONG_ALIGNMENT);

    // Freeing 1 page at byte 1. Should throw ERR_WRONG_ALIGNMENT.
    success = kpmm_free (startAddress, 1);

    EQ_SCALAR (pab[0], 0x0);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (success, EXIT_FAILURE);
    EQ_SCALAR (k_errorNumber, ERR_WRONG_ALIGNMENT);
    END();
}

TEST(PMM, double_free)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Freeing 1 page at byte 0. Already freed, so will fail.
    USYSINT startAddress = 0;
    INT success;

    success = kpmm_free (startAddress, 1);

    EQ_SCALAR (pab[0], 0x0);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (k_errorNumber, ERR_DOUBLE_FREE);
    EQ_SCALAR (success, EXIT_FAILURE);
    END();
}

TEST(PMM, double_allocate)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 0. Should be successful.
    USYSINT startAddress = 0;
    INT success;

    success = kpmm_alloc (NULL, 1, PMM_FIXED, startAddress);

    EQ_SCALAR (pab[0], 0x1);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (success, EXIT_SUCCESS);

    // Allocating again 1 page at byte 0. Should fail this time.
    success = kpmm_alloc (NULL, 1, PMM_FIXED, startAddress);

    EQ_SCALAR (pab[0], 0x1);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (success, EXIT_FAILURE);
    EQ_SCALAR (k_errorNumber, ERR_DOUBLE_ALLOC);
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
    alloc_free_fixed_misaligned_address();
    alloc_fixed_last_page();
    alloc_fixed_past_last_page();
    alloc_auto();
    autoalloc_free_autoalloc();
    autoalloc_out_of_memory();
    alloc_auto_last_page();
    double_allocate();
    double_free();
}
