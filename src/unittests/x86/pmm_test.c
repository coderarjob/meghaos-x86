#include <kernel.h>
#include <unittest/unittest.h>
#include <mock/kernel/mem.h>
#include <mock/kernel/x86/boot.h>
#include <string.h>

static U8 pab[PAB_SIZE_BYTES];

KernelErrorCodes k_errorNumber;
char *k_errorText[] = {};

Physical g_pab = PHYSICAL ((USYSINT)pab);

/* Fake Defination. At present, meghatest does not support varidac parameters. */
static bool panic_invoked;
void k_panic_ndu (const CHAR *s,...)
{
    panic_invoked = true;

    va_list l;
    va_start (l, s);

    printf ("%s", COL_RED);
    vprintf (s, l);
    printf ("%s", COL_RESET);

    va_end(l);
}

void kdebug_printf_ndu (const CHAR *fmt, ...) { }

TEST(PMM, autoalloc)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 3 pages. Address should be allocated automatically.
    USYSINT pagesCount = 3;

    Physical startAddress = kpmm_alloc (pagesCount, FALSE);

    EQ_SCALAR (startAddress.val, 4096);
    EQ_SCALAR (pab[0], 0x0E);
    EQ_SCALAR (pab[1], 0x00);

    // Allocating 3 pages again. Address should be allocated automatically.
    startAddress = kpmm_alloc (pagesCount, FALSE);

    EQ_SCALAR (startAddress.val, 4 * 4096);
    EQ_SCALAR (pab[0], 0x7E);
    EQ_SCALAR (pab[1], 0x00);
    END();
}

TEST(PMM, alloc_fixed_zero_pages)
{
    U8 shadow_pab[PAB_SIZE_BYTES];

    memset (pab, 0,PAB_SIZE_BYTES);        // Clear PAB
    memset (shadow_pab, 0,PAB_SIZE_BYTES); // Same state as PAB

    // alloc_At must fail immediately.
    INT success = kpmm_allocAt (createPhysical(4096), 0, FALSE);
    EQ_SCALAR (success, false);
    EQ_SCALAR (k_errorNumber, ERR_INVALID_ARGUMENT);
    EQ_MEM (pab, shadow_pab, PAB_SIZE_BYTES);
    END();
}

TEST(PMM, autoalloc_zero_pages)
{
    U8 shadow_pab[PAB_SIZE_BYTES];

    memset (pab, 0,PAB_SIZE_BYTES);        // Clear PAB
    memset (shadow_pab, 0,PAB_SIZE_BYTES); // Same state as PAB

    // alloc must fail immediately.
    Physical addr = kpmm_alloc (0, FALSE);
    EQ_SCALAR (isPhysicalNull(addr), TRUE);
    EQ_SCALAR (k_errorNumber, ERR_INVALID_ARGUMENT);
    EQ_MEM (pab, shadow_pab, PAB_SIZE_BYTES);

    // alloc_At must fail immediately.
    INT success = kpmm_allocAt (addr, 0, FALSE);
    EQ_SCALAR (success, false);
    EQ_SCALAR (k_errorNumber, ERR_INVALID_ARGUMENT);
    EQ_MEM (pab, shadow_pab, PAB_SIZE_BYTES);
    END();
}

TEST(PMM, free_zero_pages)
{
    U8 shadow_pab[PAB_SIZE_BYTES];

    memset (pab, 0xFF,PAB_SIZE_BYTES);          // Mark whole PAB as allocated.
    memset (shadow_pab, 0xFF, PAB_SIZE_BYTES);  // Same state as PAB.

    INT success = kpmm_free (createPhysical(0), 0);

    // free must not do anything.
    EQ_SCALAR (success, false);
    EQ_SCALAR (k_errorNumber, ERR_INVALID_ARGUMENT);
    EQ_MEM (pab, shadow_pab, PAB_SIZE_BYTES);
    END();
}


TEST(PMM, autoalloc_out_of_memory)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating every page. Then second allocation will fail.
    // Becuase the 1st page cannot be allocated, we are allocating 1 page less. Ergo the -1.
    Physical addr = kpmm_alloc (MAX_ADDRESSABLE_PAGE_COUNT - 1, FALSE);
    EQ_SCALAR (addr.val, 4096);        // Allocates from the 2nd page.
    EQ_SCALAR (pab[0], 0xFE);
    EQ_SCALAR (pab[4095], 0xFF);

    // Allocating 1 more page. Should fail as there are no free pages.
    addr = kpmm_alloc (1, FALSE);
    EQ_SCALAR (isPhysicalNull(addr), TRUE);
    EQ_SCALAR (k_errorNumber, ERR_OUT_OF_MEM);
    END();
}

TEST(PMM, autoalloc_dma_out_of_memory)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating every DMA page. Then second allocation will fail.
    // Becuase the 1st page cannot be allocated, we are allocating 1 page less. Ergo the -1.
    Physical addr = kpmm_alloc (MAX_DMA_ADDRESSABLE_PAGE_COUNT - 1, TRUE);
    EQ_SCALAR (addr.val, 4096);        // Allocates from the 2nd page.
    EQ_SCALAR (pab[0], 0xFE);
    EQ_SCALAR (pab[511], 0xFF);        // Allocated 512 pages (except the 1st page)
    EQ_SCALAR (pab[512], 0x00);

    // Allocating 1 more DMA page. Should fail as there are no free DMA pages.
    addr = kpmm_alloc (1, TRUE);
    EQ_SCALAR (isPhysicalNull(addr), TRUE);
    EQ_SCALAR (k_errorNumber, ERR_OUT_OF_MEM);

    // Allocating 1 more Non-DMA page. Should pass as there are free Non-DMA pages.
    addr = kpmm_alloc (1, FALSE);
    EQ_SCALAR (addr.val, 4096 * 4096);      // 4096th Page frame.
    EQ_SCALAR (pab[512], 0x01);
    END();
}

TEST(PMM, autoalloc_free_autoalloc)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 3 pages. Address should be allocated automatically. 
    printf ("\n:: Allocating 3 pages");
    USYSINT pagesCount = 3;
    Physical startAddress = kpmm_alloc (pagesCount, FALSE);

    EQ_SCALAR (startAddress.val, 4096);
    EQ_SCALAR (pab[0], 0x0E);
    EQ_SCALAR (pab[1], 0x00);

    // Allocating 3 pages again. Address should be allocated automatically.
    printf ("\n:: Allocating 3 pages");
    startAddress = kpmm_alloc (pagesCount, FALSE);

    EQ_SCALAR (startAddress.val, 4 * 4096);
    EQ_SCALAR (pab[0], 0x7E);
    EQ_SCALAR (pab[1], 0x00);

    // Now free 2 pages.
    printf ("\n:: Freeing 2 pages");
    INT success = kpmm_free (startAddress, 2);
    EQ_SCALAR (pab[0], 0x4E);
    EQ_SCALAR (pab[1], 0x00);
    EQ_SCALAR (success, true);

    // Allocating 3 pages again. Address should be allocated automatically. 
    printf ("\n:: Allocating 3 pages");
    startAddress = kpmm_alloc (pagesCount, FALSE);

    EQ_SCALAR (startAddress.val, 7 * 4096);
    EQ_SCALAR (pab[0], 0xCE);
    EQ_SCALAR (pab[1], 0x03);
    EQ_SCALAR (success, true);

    END();
}

TEST(PMM, alloc_fixed_past_last_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 134217728 should throw error.
    USYSINT pagesCount = 1;
    Physical startAddress = createPhysical (MAX_ADDRESSABLE_BYTE + 1);

    INT success = kpmm_allocAt (startAddress, pagesCount, FALSE);
    EQ_SCALAR (panic_invoked, false);
    EQ_SCALAR (success, false);
    EQ_SCALAR (k_errorNumber, ERR_OUTSIDE_ADDRESSABLE_RANGE);
    END();
}

TEST(PMM, alloc_auto_last_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating every but leaving the last page.  Becuase the 1st page cannot be allocated, we
    // are allocating 2 page less. Ergo the -2.
    Physical startAddress = kpmm_alloc (MAX_ADDRESSABLE_PAGE_COUNT - 2, FALSE);
    EQ_SCALAR (startAddress.val, 4096);
    EQ_SCALAR (pab[0], 0xFE);
    EQ_SCALAR (pab[4095], 0x7F);

    // Allocating 1 more page.
    startAddress = kpmm_alloc (1, FALSE);
    EQ_SCALAR (startAddress.val, MAX_ADDRESSABLE_PAGE * 4096);
    EQ_SCALAR (pab[0], 0xFE);
    EQ_SCALAR (pab[4095], 0xFF);
    END();
}

TEST(PMM, alloc_fixed_last_dma_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 134213632 (start of last addressable page) should succeed.
    USYSINT pagesCount = 1;
    Physical startAddress = createPhysical(MAX_DMA_ADDRESSABLE_PAGE * CONFIG_PAGE_FRAME_SIZE_BYTES);

    INT success = kpmm_allocAt (startAddress, pagesCount, FALSE);
    EQ_SCALAR (pab[510], 0x00);
    EQ_SCALAR (pab[511], 0x80);         // Last addressable DMA page (bit 7). Should be allocated.
    EQ_SCALAR (pab[512], 0x00);
    EQ_SCALAR (pab[4094], 0x00);
    EQ_SCALAR (pab[4095], 0x00);        // Last addressable non-DMA pages. Should be free.
    EQ_SCALAR (success, true);
    END();
}

TEST(PMM, alloc_fixed_last_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 134213632 (start of last addressable page) should succeed.
    USYSINT pagesCount = 1;
    Physical startAddress = createPhysical(MAX_ADDRESSABLE_PAGE * CONFIG_PAGE_FRAME_SIZE_BYTES);

    INT success = kpmm_allocAt (startAddress, pagesCount, FALSE);
    EQ_SCALAR (pab[4094], 0x0);
    EQ_SCALAR (pab[4095], 0x80);
    EQ_SCALAR (success, true);
    END();
}

TEST(PMM, alloc_free_fixed_misaligned_address)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 1. Should throw ERR_WRONG_ALIGNMENT.
    Physical startAddress = createPhysical(4097);

    printf ("\n:: Allocating 1 page at address 0x4097");
    INT success = kpmm_allocAt (startAddress, 1, FALSE);
    EQ_SCALAR (pab[0], 0x0);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (success, false);
    EQ_SCALAR (k_errorNumber, ERR_WRONG_ALIGNMENT);

    // Freeing 1 page at byte 1. Should throw ERR_WRONG_ALIGNMENT.
    printf ("\n:: Freeing 1 page at address 0x4097");
    success = kpmm_free (startAddress, 1);
    EQ_SCALAR (pab[0], 0x0);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (success, false);
    EQ_SCALAR (k_errorNumber, ERR_WRONG_ALIGNMENT);
    END();
}

TEST(PMM, double_free)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Freeing 1 page at location 4096. Already freed, so will fail.
    Physical startAddress = createPhysical (4096);
    INT success = kpmm_free (startAddress, 1);
    EQ_SCALAR (panic_invoked, true);
    END();
}

TEST(PMM, double_fixed_allocate)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 0. Should be successful.
    Physical addr = kpmm_alloc (1, FALSE);
    EQ_SCALAR (addr.val, 4096);
    EQ_SCALAR (pab[0], 0x2);
    EQ_SCALAR (pab[1], 0x0);

    // Allocating again 1 page at location 4096. Should fail this time.
    INT success = kpmm_allocAt (addr, 1, FALSE);
    EQ_SCALAR (pab[0], 0x2);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (success, false);
    EQ_SCALAR (k_errorNumber, ERR_DOUBLE_ALLOC);
    END();
}

TEST(PMM, alloc_fixed_2nd_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 4096. Should be successful.
    Physical startAddress = createPhysical(4096);

    INT success = kpmm_allocAt (startAddress, 1, FALSE);

    EQ_SCALAR (pab[0], 0x2);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (success, true);
    END();
}

TEST(PMM, alloc_fixed_1st_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 0. Should fail.
    Physical startAddress = createPhysical(0);

    INT success = kpmm_allocAt (startAddress, 1, FALSE);
    EQ_SCALAR (panic_invoked, true);
    END();
}

TEST(PMM, free_1st_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 0. Should fail.
    Physical startAddress = createPhysical(0);

    INT success = kpmm_free (startAddress, 1);
    EQ_SCALAR (panic_invoked, true);
    END();
}

TEST(PMM, autoalloc_excess_pages)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating MAX_ADDRESSABLE_PAGE_COUNT pages. Should fail.
    Physical startAddress = kpmm_alloc (MAX_ADDRESSABLE_PAGE_COUNT, FALSE);
    EQ_SCALAR (pab[0], 0x0);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (isPhysicalNull(startAddress), true);
    EQ_SCALAR (k_errorNumber, ERR_OUT_OF_MEM);

    END();
}

TEST(PMM, alloc_fixed_excess_pages)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating MAX_ADDRESSABLE_PAGE_COUNT pages at 4096. Should panic.
    Physical startAddress = createPhysical(4096);

    INT success = kpmm_allocAt (startAddress, MAX_ADDRESSABLE_PAGE_COUNT, FALSE);
    EQ_SCALAR (panic_invoked, false);
    EQ_SCALAR (pab[0], 0x0);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (k_errorNumber, ERR_OUTSIDE_ADDRESSABLE_RANGE);
    END();
}

TEST(PMM, free_excess_pages)
{
    // Set every page in PAB.
    memset (pab, 0xFF,PAB_SIZE_BYTES);

    // Freeing MAX_ADDRESSABLE_PAGE_COUNT pages at 4096. Should fail.
    Physical startAddress = createPhysical(4096);

    INT success = kpmm_free (startAddress, MAX_ADDRESSABLE_PAGE_COUNT);
    EQ_SCALAR (panic_invoked, false);
    EQ_SCALAR (pab[0], 0xFF);   // no change must be made to the PAB. free must fail immediately.
    EQ_SCALAR (pab[1], 0xFF);
    EQ_SCALAR (k_errorNumber, ERR_OUTSIDE_ADDRESSABLE_RANGE);
    END();
}

TEST(PMM, free_last_page)
{
    // Set every page in PAB.
    memset (pab, 0xFF,PAB_SIZE_BYTES);

    // Freeing the last page at byte 134213632 (start of last addressable page). Should succeed.
    Physical startAddress = createPhysical(MAX_ADDRESSABLE_PAGE * CONFIG_PAGE_FRAME_SIZE_BYTES);

    INT success = kpmm_free (startAddress, 1);
    EQ_SCALAR (pab[0], 0xFF);   // no change must be made to the PAB. free must fail immediately.
    EQ_SCALAR (pab[4094], 0xFF);
    EQ_SCALAR (pab[4095], 0x7F);
    END();
}

void reset()
{
    panic_invoked = false;
    resetMemFake();
    resetBootFake();
}


int main()
{
    // Will just set pointer `s_pab` to point to `pab` buffer defined here. No allocation or
    // deallocation will take place as memory map count and files count are not set and will be at
    // default zero.
    kpmm_init();

    alloc_fixed_1st_page();
    free_1st_page();

    alloc_fixed_2nd_page();

    autoalloc();
    autoalloc_out_of_memory();
    autoalloc_dma_out_of_memory();

    autoalloc_excess_pages();
    alloc_fixed_excess_pages();
    free_excess_pages();

    alloc_fixed_last_dma_page();
    alloc_fixed_last_page();
    alloc_auto_last_page();
    free_last_page();

    autoalloc_zero_pages();
    alloc_fixed_zero_pages();
    free_zero_pages();

    alloc_fixed_past_last_page();

    double_free();
    double_fixed_allocate();

    alloc_free_fixed_misaligned_address();
    autoalloc_free_autoalloc();
}
