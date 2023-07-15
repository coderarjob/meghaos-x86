#include <unittest/unittest.h>
#include <mock/kernel/mem.h>
#include <mock/kernel/x86/pmm.h>
#include <mock/common/utils.h>
#include <string.h>
#include <utils.h>
#include <moslimits.h>
#include <kerror.h>
#include <pmm.h>
#include <math.h>
#include <panic.h>

#define MAX_ACTUAL_PAGE_COUNT (BYTES_TO_PAGEFRAMES_FLOOR(kpmm_getUsableMemorySize(PMM_REGION_ANY)))

static U8 pab[PAB_SIZE_BYTES];

KernelErrorCodes k_errorNumber;
char *k_errorText[] = {};

Physical g_pab = PHYSICAL ((USYSINT)pab);

void kdebug_printf_ndu (const CHAR *fmt, ...) { }

static void init_pab()
{
    // Clear PAB.
    memset (pab, 0xFF,PAB_SIZE_BYTES);
    memset (pab, 0,ceil(MAX_ACTUAL_PAGE_COUNT/8.00));
}

TEST(PMM, autoalloc)
{
    // Allocating 3 pages. Address should be allocated automatically.
    USYSINT pagesCount = 3;

    Physical start;
    EQ_SCALAR (true, kpmm_alloc (&start, pagesCount, PMM_REGION_ANY));
    EQ_SCALAR (start.val, 0);
    EQ_SCALAR (pab[0], 0x07);
    EQ_SCALAR (pab[1], 0x00);

    // Allocating 3 pages again. Address should be allocated automatically.
    EQ_SCALAR(true, kpmm_alloc (&start, pagesCount, PMM_REGION_ANY));
    EQ_SCALAR (start.val, 3 * 4096);
    EQ_SCALAR (pab[0], 0x3F);
    EQ_SCALAR (pab[1], 0x00);
    END();
}

TEST(PMM, alloc_fixed_zero_pages)
{
    U8 shadow_pab[PAB_SIZE_BYTES];

    memset (pab, 0,PAB_SIZE_BYTES);        // Clear PAB
    memset (shadow_pab, 0,PAB_SIZE_BYTES); // Same state as PAB

    // alloc_At must fail immediately.
    EQ_SCALAR (false, kpmm_allocAt (createPhysical(4096), 0, PMM_REGION_ANY));
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
    Physical addr;
    EQ_SCALAR(false, kpmm_alloc (&addr, 0, PMM_REGION_ANY));
    EQ_SCALAR (k_errorNumber, ERR_INVALID_ARGUMENT);
    EQ_MEM (pab, shadow_pab, PAB_SIZE_BYTES);

    // alloc_At must fail immediately.
    EQ_SCALAR (false, kpmm_allocAt (addr, 0, PMM_REGION_ANY));
    EQ_SCALAR (k_errorNumber, ERR_INVALID_ARGUMENT);
    EQ_MEM (pab, shadow_pab, PAB_SIZE_BYTES);
    END();
}

TEST(PMM, free_zero_pages)
{
    U8 shadow_pab[PAB_SIZE_BYTES];

    memset (pab, 0xFF,PAB_SIZE_BYTES);          // Mark whole PAB as allocated.
    memset (shadow_pab, 0xFF, PAB_SIZE_BYTES);  // Same state as PAB.

    // free must not do anything.
    EQ_SCALAR (false, kpmm_free (createPhysical(0), 0));
    EQ_SCALAR (k_errorNumber, ERR_INVALID_ARGUMENT);
    EQ_MEM (pab, shadow_pab, PAB_SIZE_BYTES);
    END();
}


TEST(PMM, autoalloc_out_of_memory)
{
    // Simulating system of 5MB of RAM.
    // page Count = 1280
    // PAB byte = (1279 / 8) = 159
    // PAB bit = (1279 % 8)  = 7
    kpmm_arch_getInstalledMemoryByteCount_fake.ret = 5 * MB;
    init_pab();

    // Allocating every page. Then second allocation will fail.
    Physical addr;
    EQ_SCALAR(true, kpmm_alloc (&addr, MAX_ACTUAL_PAGE_COUNT, PMM_REGION_ANY));
    EQ_SCALAR (addr.val, 0);        // Allocates from the 1st page.

    // Allocating 1 more page. Should fail as there are no free pages.
    EQ_SCALAR(false, kpmm_alloc (&addr, 1, PMM_REGION_ANY));
    EQ_SCALAR (k_errorNumber, ERR_OUT_OF_MEM);

    // PAB in the end
    EQ_SCALAR (pab[0], 0xFF);
    EQ_SCALAR (pab[159], 0xFF);
    EQ_SCALAR (pab[160], 0xFF);
    END();
}

/*TEST(PMM, autoalloc_dma_out_of_memory)
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
}*/

TEST(PMM, autoalloc_free_autoalloc)
{
    // Allocating 3 pages. Address should be allocated automatically.
    printf ("\n:: Allocating 3 pages");
    Physical start;

    EQ_SCALAR(true, kpmm_alloc (&start, 3, PMM_REGION_ANY));
    EQ_SCALAR (start.val, 0);
    EQ_SCALAR (pab[0], 0x07);
    EQ_SCALAR (pab[1], 0x00);

    // Allocating 3 pages again. Address should be allocated automatically.
    printf ("\n:: Allocating 3 pages");
    EQ_SCALAR(true, kpmm_alloc (&start, 3, PMM_REGION_ANY));
    EQ_SCALAR (start.val, 3 * 4096);
    EQ_SCALAR (pab[0], 0x3F);
    EQ_SCALAR (pab[1], 0x00);

    // Now free 2 pages.
    printf ("\n:: Freeing 2 pages");
    EQ_SCALAR(true, kpmm_free (start, 2));
    EQ_SCALAR (pab[0], 0x27);
    EQ_SCALAR (pab[1], 0x00);

    // Allocating 3 pages again. Address should be allocated automatically.
    printf ("\n:: Allocating 3 pages");
    EQ_SCALAR(true, kpmm_alloc (&start, 3, PMM_REGION_ANY));
    EQ_SCALAR (start.val, 6 * 4096);
    EQ_SCALAR (pab[0], 0xE7);
    EQ_SCALAR (pab[1], 0x01);

    END();
}

TEST(PMM, alloc_fixed_past_last_page)
{
    // Total system RAM is 5 MB.
    // There are 1280 pages (numbered 0 to 1279). Here the test tries to allocate the 1280th page.
    // Thus should panic and fail.
    kpmm_arch_getInstalledMemoryByteCount_fake.ret = 5 * MB;
    init_pab();

    USYSINT pagesCount = 1;
    Physical start = createPhysical (MAX_ACTUAL_PAGE_COUNT * CONFIG_PAGE_FRAME_SIZE_BYTES);

    EQ_SCALAR (false, kpmm_allocAt (start, pagesCount, PMM_REGION_ANY));
    EQ_SCALAR (panic_invoked, false);
    EQ_SCALAR (k_errorNumber, ERR_OUTSIDE_ADDRESSABLE_RANGE);
    END();
}

/*TEST(PMM, alloc_fixed_past_last_dma_page)
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
}*/

TEST(PMM, alloc_auto_last_page)
{
    // Total system RAM is 5 MB.
    kpmm_arch_getInstalledMemoryByteCount_fake.ret = 5 * MB;
    init_pab();

    // Allocating every but leaving the last page.
    // Last page is 1279, which is 159:7 bit in the PAB.
    Physical start;
    EQ_SCALAR(true, kpmm_alloc (&start, MAX_ACTUAL_PAGE_COUNT - 1, PMM_REGION_ANY));
    EQ_SCALAR (start.val, 0);
    EQ_SCALAR (pab[0], 0xFF);
    EQ_SCALAR (pab[159], 0x7F);

    // Allocating 1 more page.
    EQ_SCALAR(true, kpmm_alloc (&start, 1, PMM_REGION_ANY));
    EQ_SCALAR (start.val, 1279 * 4096);
    EQ_SCALAR (pab[0], 0xFF);
    EQ_SCALAR (pab[159], 0xFF);
    END();
}

/*TEST(PMM, alloc_fixed_last_dma_page)
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
}*/

TEST(PMM, alloc_fixed_last_page)
{
    // Total system RAM is 5 MB.
    kpmm_arch_getInstalledMemoryByteCount_fake.ret = 5 * MB;
    init_pab();

    // Allocating the last actual physical page. Here that is the 1280th page.
    // 1280th page is 159:7 bit in the PAB.
    USYSINT pagesCount = 1;
    USYSINT lastPageAddress = (MAX_ACTUAL_PAGE_COUNT - 1) * CONFIG_PAGE_FRAME_SIZE_BYTES;
    Physical startAddress = createPhysical(lastPageAddress);

    bool success = kpmm_allocAt (startAddress, pagesCount, PMM_REGION_ANY);
    EQ_SCALAR (pab[160], 0xFF);
    EQ_SCALAR (pab[159], 0x80);
    EQ_SCALAR (success, true);
    END();
}

TEST(PMM, alloc_free_fixed_misaligned_address)
{
    // Allocating 1 page at byte 4097. Should throw ERR_WRONG_ALIGNMENT.
    Physical startAddress = createPhysical(4097);

    printf ("\n:: Allocating 1 page at address 0x4097");
    bool success = kpmm_allocAt (startAddress, 1, PMM_REGION_ANY);
    EQ_SCALAR (pab[0], 0x0);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (success, false);
    EQ_SCALAR (k_errorNumber, ERR_WRONG_ALIGNMENT);

    // Freeing 1 page at byte 4097. Should throw ERR_WRONG_ALIGNMENT.
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
    // Freeing 1 page at location 0. Already freed, so will fail.
    kpmm_free (createPhysical(0), 1);
    EQ_SCALAR (panic_invoked, true);
    END();
}

TEST(PMM, double_fixed_allocate)
{
    // Allocating 1 page at byte 0. Should be successful.
    Physical addr;
    EQ_SCALAR(true, kpmm_alloc (&addr, 1, PMM_REGION_ANY));
    EQ_SCALAR (addr.val, 0);
    EQ_SCALAR (pab[0], 0x1);
    EQ_SCALAR (pab[1], 0x0);

    // Allocating again 1 page at the address returned. Should fail this time.
    EQ_SCALAR(false, kpmm_allocAt (addr, 1, PMM_REGION_ANY));
    EQ_SCALAR (pab[0], 0x1);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (k_errorNumber, ERR_DOUBLE_ALLOC);
    END();
}

TEST(PMM, alloc_fixed_2nd_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 4096. Should pass.
    Physical start = createPhysical(4096);

    EQ_SCALAR(true, kpmm_allocAt (start, 1, PMM_REGION_ANY));
    EQ_SCALAR (pab[0], 0x2);
    EQ_SCALAR (pab[1], 0x0);
    END();
}

TEST(PMM, alloc_fixed_1st_page)
{
    // Clear PAB.
    memset (pab, 0,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 0. Should pass.
    Physical start = createPhysical(0);

    EQ_SCALAR(true, kpmm_allocAt (start, 1, PMM_REGION_ANY));
    END();
}

TEST(PMM, free_1st_page)
{
    // Clear PAB.
    memset (pab, 0xFF,PAB_SIZE_BYTES);

    // Allocating 1 page at byte 0. Should pass.
    Physical start = createPhysical(0);
    EQ_SCALAR(true, kpmm_free (start, 1));
    END();
}

TEST(PMM, autoalloc_excess_pages)
{
    // Allocating more pages than present. Should fail.
    Physical start;
    EQ_SCALAR(false, kpmm_alloc (&start, MAX_ACTUAL_PAGE_COUNT + 1, PMM_REGION_ANY));
    EQ_SCALAR (pab[0], 0x0);
    EQ_SCALAR (pab[1], 0x0);
    EQ_SCALAR (k_errorNumber, ERR_OUT_OF_MEM);

    END();
}

TEST(PMM, alloc_fixed_excess_pages)
{
    // Allocating every physical page. Should panic.
    Physical start = createPhysical(0);

    EQ_SCALAR(false, kpmm_allocAt (start, MAX_ACTUAL_PAGE_COUNT + 1, PMM_REGION_ANY));
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
    Physical startAddress = createPhysical(0);

    EQ_SCALAR (false, kpmm_free (startAddress, MAX_ACTUAL_PAGE_COUNT + 1));
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
    kpmm_arch_getInstalledMemoryByteCount_fake.ret = 5 * MB;

    // Freeing the last physical page (5 MB of RAM). Should pass.
    // Last page is 1279, which is 159:7 bit in the PAB.
    USYSINT lastPageAddress = (MAX_ACTUAL_PAGE_COUNT - 1) * CONFIG_PAGE_FRAME_SIZE_BYTES;
    Physical startAddress = createPhysical(lastPageAddress);

    EQ_SCALAR (true, kpmm_free (startAddress, 1));
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
    // 5MB of RAM
    kpmm_arch_getInstalledMemoryByteCount_fake.ret = 5 * MB;
    init_pab();

    // Allocate whole RAM
    // 5 MB is 1280 physical pages. Last page is 159:7 bit in PAB.
    Physical start;
    EQ_SCALAR(true, kpmm_alloc (&start, MAX_ACTUAL_PAGE_COUNT, PMM_REGION_ANY));
    EQ_SCALAR (start.val, 0);
    EQ_SCALAR (pab[0], 0xFF);
    EQ_SCALAR (pab[159], 0xFF);
    EQ_SCALAR (pab[160], 0xFF); // Beyond 5 MB memory (should be not free)

    size_t available_ram = kpmm_getFreeMemorySize ();
    EQ_SCALAR (available_ram, 0);

    END();
}

UINT powerOfTwo(UINT e)
{
    return pow(2, e);
}

void reset()
{
    panic_invoked = false;
    resetX86Pmm();
    resetUtilsFake();

    power_of_two_fake.handler = powerOfTwo;

    // Default size of RAM is set to 2 MB.
    kpmm_arch_getInstalledMemoryByteCount_fake.ret = 2 * MB;

    init_pab();
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
    //autoalloc_dma_out_of_memory();

    autoalloc_excess_pages();
    alloc_fixed_excess_pages();
    free_excess_pages();

    //alloc_fixed_last_dma_page();
    alloc_fixed_last_page();
    alloc_auto_last_page();
    free_last_page();

    autoalloc_zero_pages();
    alloc_fixed_zero_pages();
    free_zero_pages();

    alloc_fixed_past_last_page();
    //alloc_fixed_past_last_dma_page();

    // TODO: Why is double free panics but not double alloc?
    double_free();
    double_fixed_allocate();

    alloc_free_fixed_misaligned_address();
    autoalloc_free_autoalloc();

    free_mem_size_mem_full();
    free_mem_size();
    free_mem_size_free_last_page();
    free_mem_size_after_autoalloc();
}
