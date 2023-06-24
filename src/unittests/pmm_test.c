#include <unittest/unittest.h>
#include <mock/kernel/mem.h>
#include <mock/kernel/x86/pmm.h>
#include <string.h>
#include <utils.h>
#include <moslimits.h>
#include <kerror.h>
#include <pmm.h>

#define MAX_ACTUAL_PAGE_COUNT (kpmm_getAddressablePageCount (false))

static U8 pab[PAB_SIZE_BYTES];

KernelErrorCodes k_errorNumber;
char *k_errorText[] = {};

Physical g_pab = PHYSICAL ((USYSINT)pab);

void kdebug_printf_ndu (const CHAR *fmt, ...) { }

TEST(PMM, autoalloc)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 3 pages. Address should be allocated automatically.
    USYSINT pagesCount = 3;

    Physical startAddress = kpmm_alloc (pagesCount, false);

    EQ_SCALAR (startAddress.val, 4096);
    EQ_SCALAR (pab[0], 0x0E);
    EQ_SCALAR (pab[1], 0x00);

    // Allocating 3 pages again. Address should be allocated automatically.
    startAddress = kpmm_alloc (pagesCount, false);

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
    bool success = kpmm_allocAt (createPhysical(4096), 0, false);
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
    Physical addr = kpmm_alloc (0, false);
    EQ_SCALAR (isPhysicalNull(addr), true);
    EQ_SCALAR (k_errorNumber, ERR_INVALID_ARGUMENT);
    EQ_MEM (pab, shadow_pab, PAB_SIZE_BYTES);

    // alloc_At must fail immediately.
    bool success = kpmm_allocAt (addr, 0, false);
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

    bool success = kpmm_free (createPhysical(0), 0);

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

    // Simulating system of 5MB of RAM.
    // page Count = 1280
    // PAB byte = (1279 / 8) = 159
    // PAB bit = (1279 % 8)  = 7
    kpmm_getAddressableByteCount_fake.ret = 5 * MB;

    // Allocating every page. Then second allocation will fail.
    // Becuase the 1st page cannot be allocated, we are allocating 1 page less. Ergo the -1.
    Physical addr = kpmm_alloc (MAX_ACTUAL_PAGE_COUNT - 1, false);

    EQ_SCALAR (addr.val, 4096);        // Allocates from the 2nd page.

    // Allocating 1 more page. Should fail as there are no free pages.
    addr = kpmm_alloc (1, false);
    EQ_SCALAR (isPhysicalNull(addr), true);
    EQ_SCALAR (k_errorNumber, ERR_OUT_OF_MEM);

    // PAB in the end
    EQ_SCALAR (pab[0], 0xFE);
    EQ_SCALAR (pab[159], 0xFF);
    EQ_SCALAR (pab[160], 0x00);
    END();
}

TEST(PMM, autoalloc_dma_out_of_memory)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Simulating system of 50 MB of RAM (which is more then the 16 MB DMA addressable).
    // Max DMA addressable RAM = 16777216 bytes
    // page Count = 4096
    // PAB byte = (4095 / 8) = 511
    // PAB bit = (4095 % 8)  = 7

    // Assuming system of 50 MB of RAM (which is more then the 16 MB DMA addressable).
    // Because we are passing isDMA = true, in kpmm_alloc, total addressable byte is =
    // MAX_PAB_DMA_ADDRESSABLE_BYTE_COUNT.
    kpmm_getAddressableByteCount_fake.ret = MAX_PAB_DMA_ADDRESSABLE_BYTE_COUNT;

    // Allocating every DMA page. Then second allocation will fail.
    // Becuase the 1st page cannot be allocated, we are allocating 1 page less. Ergo the -1.
    Physical addr = kpmm_alloc (MAX_PAB_DMA_ADDRESSABLE_PAGE_COUNT - 1, true);
    EQ_SCALAR (addr.val, 4096);        // Allocates from the 2nd page.

    // Allocating 1 more DMA page. Should fail as there are no free DMA pages.
    addr = kpmm_alloc (1, true);
    EQ_SCALAR (isPhysicalNull(addr), true);
    EQ_SCALAR (k_errorNumber, ERR_OUT_OF_MEM);

    EQ_SCALAR (pab[0], 0xFE);
    EQ_SCALAR (pab[511], 0xFF);        // Allocated 512 pages (except the 1st page)
    EQ_SCALAR (pab[512], 0x00);

    // Allocating 1 more Non-DMA page. Should pass as there are free Non-DMA pages.

    // Assuming system of 50 MB of RAM (which is more then the 16 MB DMA addressable).
    // Because we are passing isDMA = false, in kpmm_alloc, total addressable byte is = system RAM.
    kpmm_getAddressableByteCount_fake.ret = 50 * MB;

    addr = kpmm_alloc (1, false);
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
    Physical startAddress = kpmm_alloc (pagesCount, false);

    EQ_SCALAR (startAddress.val, 4096);
    EQ_SCALAR (pab[0], 0x0E);
    EQ_SCALAR (pab[1], 0x00);

    // Allocating 3 pages again. Address should be allocated automatically.
    printf ("\n:: Allocating 3 pages");
    startAddress = kpmm_alloc (pagesCount, false);

    EQ_SCALAR (startAddress.val, 4 * 4096);
    EQ_SCALAR (pab[0], 0x7E);
    EQ_SCALAR (pab[1], 0x00);

    // Now free 2 pages.
    printf ("\n:: Freeing 2 pages");
    bool success = kpmm_free (startAddress, 2);
    EQ_SCALAR (pab[0], 0x4E);
    EQ_SCALAR (pab[1], 0x00);
    EQ_SCALAR (success, true);

    // Allocating 3 pages again. Address should be allocated automatically.
    printf ("\n:: Allocating 3 pages");
    startAddress = kpmm_alloc (pagesCount, false);

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

    // Total system RAM is 5 MB.
    // There are 1280 pages (numbered 0 to 1279). Here the test tries to allocate the 1280th page.
    // Thus should panic and fail.
    kpmm_getAddressableByteCount_fake.ret = 5 * MB;

    USYSINT pagesCount = 1;
    Physical startAddress = createPhysical (MAX_ACTUAL_PAGE_COUNT * CONFIG_PAGE_FRAME_SIZE_BYTES);

    bool success = kpmm_allocAt (startAddress, pagesCount, false);
    EQ_SCALAR (panic_invoked, false);
    EQ_SCALAR (success, false);
    EQ_SCALAR (k_errorNumber, ERR_OUTSIDE_ADDRESSABLE_RANGE);
    END();
}

TEST(PMM, alloc_fixed_past_last_dma_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // We are passing isDMA = true, in kpmm_allocAt, total addressable byte is =
    // MAX_PAB_DMA_ADDRESSABLE_BYTE_COUNT.
    kpmm_getAddressableByteCount_fake.ret = 16 * MB;

    USYSINT pagesCount = 1;
    Physical startAddr = createPhysical ((MAX_PAB_DMA_ADDRESSABLE_PAGE + 1) * CONFIG_PAGE_FRAME_SIZE_BYTES);

    bool success = kpmm_allocAt (startAddr, pagesCount, true);
    EQ_SCALAR (panic_invoked, false);
    EQ_SCALAR (success, false);
    EQ_SCALAR (k_errorNumber, ERR_OUTSIDE_ADDRESSABLE_RANGE);
    END();
}

TEST(PMM, alloc_auto_last_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Total system RAM is 5 MB.
    kpmm_getAddressableByteCount_fake.ret = 5 * MB;

    // Allocating every but leaving the last page.  Becuase the 1st page cannot be allocated, we
    // are allocating 2 page less. Ergo the -2.
    // Last page is 1279, which is 159:7 bit in the PAB.

    Physical startAddress = kpmm_alloc (MAX_ACTUAL_PAGE_COUNT - 2, false);
    EQ_SCALAR (startAddress.val, 4096);
    EQ_SCALAR (pab[0], 0xFE);
    EQ_SCALAR (pab[159], 0x7F);

    // Allocating 1 more page.
    startAddress = kpmm_alloc (1, false);
    EQ_SCALAR (startAddress.val, 1279 * 4096);
    EQ_SCALAR (pab[0], 0xFE);
    EQ_SCALAR (pab[159], 0xFF);
    END();
}

TEST(PMM, alloc_fixed_last_dma_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Total system RAM is 50 MB (which is more than the 16 MB of DMA accessable RAM)
    // Because we are passing isDMA = false, in kpmm_alloc, total addressable byte is = system RAM.
    kpmm_getAddressableByteCount_fake.ret = 50 * MB;

    // Allocating the last DMA page. Here that is the 4096th page. Should succeed.
    // 4096th page is 511:7 bit in the PAB.
    USYSINT pagesCount = 1;
    USYSINT lastDMAPageAddress = MAX_PAB_DMA_ADDRESSABLE_PAGE * CONFIG_PAGE_FRAME_SIZE_BYTES;
    Physical startAddress = createPhysical(lastDMAPageAddress);

    bool success = kpmm_allocAt (startAddress, pagesCount, false);
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

    // Total system RAM is 5 MB.
    kpmm_getAddressableByteCount_fake.ret = 5 * MB;

    // Allocating the last actual physical page. Here that is the 1280th page.
    // 1280th page is 159:7 bit in the PAB.
    USYSINT pagesCount = 1;
    USYSINT lastPageAddress = (MAX_ACTUAL_PAGE_COUNT - 1) * CONFIG_PAGE_FRAME_SIZE_BYTES;
    Physical startAddress = createPhysical(lastPageAddress);

    bool success = kpmm_allocAt (startAddress, pagesCount, false);
    EQ_SCALAR (pab[160], 0x0);
    EQ_SCALAR (pab[159], 0x80);
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
    bool success = kpmm_allocAt (startAddress, 1, false);
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
    kpmm_free (startAddress, 1);
    EQ_SCALAR (panic_invoked, true);
    END();
}

TEST(PMM, double_fixed_allocate)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 0. Should be successful.
    Physical addr = kpmm_alloc (1, false);
    EQ_SCALAR (addr.val, 4096);
    EQ_SCALAR (pab[0], 0x2);
    EQ_SCALAR (pab[1], 0x0);

    // Allocating again 1 page at location 4096. Should fail this time.
    bool success = kpmm_allocAt (addr, 1, false);
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

    bool success = kpmm_allocAt (startAddress, 1, false);

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

    kpmm_allocAt (startAddress, 1, false);
    EQ_SCALAR (panic_invoked, true);
    END();
}

TEST(PMM, free_1st_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 0. Should fail.
    Physical startAddress = createPhysical(0);

    kpmm_free (startAddress, 1);
    EQ_SCALAR (panic_invoked, true);
    END();
}

TEST(PMM, autoalloc_excess_pages)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating more pages than present. Should fail.
    Physical startAddress = kpmm_alloc (MAX_ACTUAL_PAGE_COUNT, false);
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

    // Allocating every physical page, stating at 4096. Should panic.
    Physical startAddress = createPhysical(4096);

    bool success = kpmm_allocAt (startAddress, MAX_ACTUAL_PAGE_COUNT, false);
    EQ_SCALAR (success, false);
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

    // Freeing more pages than available. Should fail.
    Physical startAddress = createPhysical(4096);

    bool success = kpmm_free (startAddress, MAX_ACTUAL_PAGE_COUNT);
    EQ_SCALAR (success, false);
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

    // System has 5 MB of ram
    kpmm_getAddressableByteCount_fake.ret = 5 * MB;

    // Freeing the last physical page (5 MB of RAM). Should succeed.
    // Last page is 1279, which is 159:7 bit in the PAB.
    USYSINT lastPageAddress = (MAX_ACTUAL_PAGE_COUNT - 1) * CONFIG_PAGE_FRAME_SIZE_BYTES;
    Physical startAddress = createPhysical(lastPageAddress);

    bool success = kpmm_free (startAddress, 1);
    EQ_SCALAR (success, true);
    EQ_SCALAR (pab[0], 0xFF);   // no change must be made to the PAB. free must fail immediately.
    EQ_SCALAR (pab[158], 0xFF);
    EQ_SCALAR (pab[159], 0x7F);
    EQ_SCALAR (pab[160], 0xFF);
    END();
}

TEST(PMM, free_mem_size_mem_full)
{
    // Set every page in PAB.
    memset (pab, 0xFF,PAB_SIZE_BYTES);
    size_t freeMemSize = kpmm_getFreeMemorySize ();
    EQ_SCALAR (freeMemSize, 0);

    END();
}

TEST(PMM, free_mem_size)
{
    // Set every page in PAB.
    memset (pab, 0xFF,PAB_SIZE_BYTES);

    // Free 3 pages starting from address 4096
    printf ("\n:: Freeing 3 pages");
    Physical startAddress = createPhysical (4096);
    bool success = kpmm_free (startAddress, 3);
    EQ_SCALAR (success, true);

    size_t freeMemSize = kpmm_getFreeMemorySize ();
    EQ_SCALAR (freeMemSize, 4096 * 3);

    END();
}

TEST(PMM, free_mem_size_free_last_page)
{
    // Set every page in PAB.
    memset (pab, 0xFF,PAB_SIZE_BYTES);

    // Freeing the last physical page (5 MB of RAM). Should succeed.
    USYSINT lastPageAddress = (MAX_ACTUAL_PAGE_COUNT - 1) * CONFIG_PAGE_FRAME_SIZE_BYTES;
    Physical startAddress = createPhysical(lastPageAddress);

    bool success = kpmm_free (startAddress, 1);
    EQ_SCALAR (success, true);

    size_t freeMemSize = kpmm_getFreeMemorySize ();
    EQ_SCALAR (freeMemSize, 4096);

    END();
}

TEST(PMM, free_mem_size_after_autoalloc)
{
    // Clear every page in PAB.
    memset (pab, 0x00,PAB_SIZE_BYTES);

    // 5MB of RAM
    kpmm_getAddressableByteCount_fake.ret = 5 * MB;

    // Allocate whole RAM
    // 5 MB is 1280 physical pages. Last page is 159:7 bit in PAB.
    Physical startAddress = kpmm_alloc (MAX_ACTUAL_PAGE_COUNT - 1, false);
    EQ_SCALAR (startAddress.val, 4096);     // Allocation starts from the 2nd page.
    EQ_SCALAR (pab[0], 0xFE);
    EQ_SCALAR (pab[159], 0xFF);
    EQ_SCALAR (pab[160], 0x00);

    size_t available_ram = kpmm_getFreeMemorySize ();
    EQ_SCALAR (available_ram, 0);

    END();
}

void reset()
{
    panic_invoked = false;
    resetMemFake();
    resetX86Pmm();

    // Default size of RAM is set to 2 MB.
    kpmm_getAddressableByteCount_fake.ret = 2 * MB;
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
    alloc_fixed_past_last_dma_page();

    double_free();
    double_fixed_allocate();

    alloc_free_fixed_misaligned_address();
    autoalloc_free_autoalloc();

    free_mem_size_mem_full();
    free_mem_size();
    free_mem_size_free_last_page();
    free_mem_size_after_autoalloc();
}
