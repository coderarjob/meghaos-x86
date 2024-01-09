#include <unittest/unittest.h>
#include <mock/kernel/mem.h>
#include <mock/kernel/x86/pmm.h>
#include <mock/common/utils.h>
#include <string.h>
#include <utils.h>
#include <moslimits.h>
#include <paging.h>
#include <kerror.h>
#include <pmm.h>
#include <math.h>
#include <panic.h>

#define ACTUAL_MEMORY_SIZE    (kpmm_getUsableMemorySize (PMM_REGION_ANY))
#define MAX_ACTUAL_PAGE_COUNT (BYTES_TO_PAGEFRAMES_FLOOR (ACTUAL_MEMORY_SIZE))

#define BITS_PER_STATE  2U
#define STATES_PER_BYTE (8U / BITS_PER_STATE)
#define MAX_STATE       (power_of_two (BITS_PER_STATE) - 1)

#define PAB_BYTE(addr) ((addr) / (STATES_PER_BYTE))
#define PAB_BIT(addr)  (((addr) % (STATES_PER_BYTE)) * BITS_PER_STATE)

static U8 pab[PAB_SIZE_BYTES];
Physical g_pab = PHYSICAL ((USYSINT)pab);

KernelErrorCodes k_errorNumber;
char *k_errorText[] = {};

void kdebug_printf_ndu (const CHAR *fmt, ...) {}
static void validate_pab (const U8 *pab, USYSINT addr, KernelPhysicalMemoryStates state);
static void set_pab (U8 *const pab, USYSINT start, UINT pgCount, KernelPhysicalMemoryStates state);

/*
 *
 * TEST CASES
 * =================================================================================================
 * Common:
 * 1. Page count == 0 | ERR_INVALID_ARGUMENT | zero_page_count
 * 2. Not initialized | panic                | pab_uninitialized
 *
 * kpmm_free:
 * 1. Start address not page aligned     | ERR_WRONG_ALIGNMENT          | free_allocat_misaligned
 * 2. Start addr + pg count > Usable RAM | ERR_OUTSIDE_ADDRESSABLE_RANGE| free_allocat_outsideRange
 * 3. Free pages previously marked used  | Success                      | free_success
 * 4. Free already free pages            | panic                        | free_doubleFree
 * 5. Free reserved pages                | panic                        | free_reservePages
 *
 * kpmm_alloc:
 * 1. No page is free     | ERR_OUT_OF_MEM | alloc_allocAt_outOfMem
 * 2. Some pages are free | Success        | alloc_success
 *
 * kpmm_allocAt:
 * 1. Start address not page aligned     | ERR_WRONG_ALIGNMENT          | free_allocat_misaligned
 * 2. Start addr + pg count > Usable RAM | ERR_OUTSIDE_ADDRESSABLE_RANGE| free_allocat_outsideRange
 * 3. Not enough free pages at addr      | ERR_DOUBLE_ALLOC             | alloc_allocAt_outOfMem
 * 4. Some pages are free                | Success                      | allocAt_success
 */

static void init_pab()
{
    // Clear PAB.
    set_pab (pab, 0, MAX_PAB_ADDRESSABLE_PAGE_COUNT, PMM_STATE_INVALID);
    set_pab (pab, 0, MAX_ACTUAL_PAGE_COUNT, PMM_STATE_FREE);
}

#ifdef DEBUG
TEST (PMM, pab_uninitialized_mustfail)
{
    Physical addr = createPhysical (CONFIG_PAGE_FRAME_SIZE_BYTES);
    kpmm_free (addr, 1);
    EQ_SCALAR (panic_invoked, true);

    panic_invoked = false;
    kpmm_allocAt (addr, 1, PMM_REGION_ANY);
    EQ_SCALAR (panic_invoked, true);

    panic_invoked = false;
    kpmm_alloc (&addr, 1, PMM_REGION_ANY);
    EQ_SCALAR (panic_invoked, true);

    END();
}
#endif

TEST (PMM, zero_page_count)
{
    Physical addr = createPhysical (CONFIG_PAGE_FRAME_SIZE_BYTES);
    EQ_SCALAR (false, kpmm_free (addr, 0));
    EQ_SCALAR (k_errorNumber, ERR_INVALID_ARGUMENT);

    EQ_SCALAR (false, kpmm_allocAt (addr, 0, PMM_REGION_ANY));
    EQ_SCALAR (k_errorNumber, ERR_INVALID_ARGUMENT);

    EQ_SCALAR (false, kpmm_alloc (&addr, 0, PMM_REGION_ANY));
    EQ_SCALAR (k_errorNumber, ERR_INVALID_ARGUMENT);
    END();
}

TEST (PMM, free_allocat_misaligned)
{
    Physical addr = createPhysical (CONFIG_PAGE_FRAME_SIZE_BYTES + 1);
    EQ_SCALAR (false, kpmm_free (addr, 1));
    EQ_SCALAR (k_errorNumber, ERR_WRONG_ALIGNMENT);

    EQ_SCALAR (false, kpmm_allocAt (addr, 1, PMM_REGION_ANY));
    EQ_SCALAR (k_errorNumber, ERR_WRONG_ALIGNMENT);

    END();
}

TEST (PMM, free_allocat_outsideRange)
{
    struct tuple
    {
        Physical addr;
        UINT pageCount;
    } failureCases[] = {
        { createPhysical (0), MAX_ACTUAL_PAGE_COUNT + 1 },
        { createPhysical (ACTUAL_MEMORY_SIZE), 1 },
        { createPhysical (ACTUAL_MEMORY_SIZE - CONFIG_PAGE_FRAME_SIZE_BYTES), 2 },
    };

    for (int i = 0; i < ARRAY_LENGTH (failureCases); i++)
    {
        struct tuple *t = &failureCases[i];

        printf ("\n:: With address %x, page count: %d", t->addr.val, t->pageCount);
        EQ_SCALAR (false, kpmm_free (t->addr, t->pageCount));
        EQ_SCALAR (k_errorNumber, ERR_OUTSIDE_ADDRESSABLE_RANGE);

        EQ_SCALAR (false, kpmm_allocAt (t->addr, t->pageCount, PMM_REGION_ANY));
        EQ_SCALAR (k_errorNumber, ERR_OUTSIDE_ADDRESSABLE_RANGE);
    }

    END();
}

TEST (PMM, allocat_success)
{
    Physical addresses[] = {
        createPhysical (0),
        createPhysical (ACTUAL_MEMORY_SIZE - 2 * CONFIG_PAGE_FRAME_SIZE_BYTES),
    };

    // Allocates two consecutive addresses
    for (int i = 0; i < ARRAY_LENGTH (addresses); i++)
    {
        Physical addr = addresses[i];
        printf ("\n:: Allocating x:%x, %x", addr.val, addr.val + CONFIG_PAGE_FRAME_SIZE_BYTES);

        EQ_SCALAR (true, kpmm_allocAt (addr, 2, PMM_REGION_ANY));

        validate_pab (pab, addr.val, PMM_STATE_USED);
        validate_pab (pab, addr.val + CONFIG_PAGE_FRAME_SIZE_BYTES, PMM_STATE_USED);
    }

    END();
}

TEST (PMM, alloc_allocAt_outOfMem)
{
    // Only the first two and last two pages are free.
    set_pab (pab, 0, MAX_ACTUAL_PAGE_COUNT, PMM_STATE_USED);

    Physical addr = createPhysical (0);
    EQ_SCALAR (false, kpmm_alloc (&addr, 2, PMM_REGION_ANY));
    EQ_SCALAR (k_errorNumber, ERR_OUT_OF_MEM);

    EQ_SCALAR (false, kpmm_allocAt (addr, 2, PMM_REGION_ANY));
    EQ_SCALAR (k_errorNumber, ERR_DOUBLE_ALLOC);
    END();
}

TEST (PMM, alloc_success)
{
    // Only the first two and last two pages are free.
    set_pab (pab, 0, MAX_ACTUAL_PAGE_COUNT, PMM_STATE_USED);
    set_pab (pab, 0, 2, PMM_STATE_FREE);
    set_pab (pab, ACTUAL_MEMORY_SIZE - 2 * 4096, 2, PMM_STATE_FREE);

    Physical expectedAddrs[] = {
        createPhysical (0),
        createPhysical (ACTUAL_MEMORY_SIZE - 2 * CONFIG_PAGE_FRAME_SIZE_BYTES),
    };

    for (int i = 0; i < ARRAY_LENGTH (expectedAddrs); i++)
    {
        Physical expAddr = expectedAddrs[i];
        printf ("\n:: Should allocate x:%x, %x", expAddr.val,
                expAddr.val + CONFIG_PAGE_FRAME_SIZE_BYTES);

        Physical addr;
        EQ_SCALAR (true, kpmm_alloc (&addr, 2, PMM_REGION_ANY));
        EQ_SCALAR (addr.val, expAddr.val);

        validate_pab (pab, addr.val, PMM_STATE_USED);
        validate_pab (pab, addr.val + CONFIG_PAGE_FRAME_SIZE_BYTES, PMM_STATE_USED);
    }

    END();
}

TEST (PMM, free_reservePages)
{
    // Set every page as Reserved.
    set_pab (pab, 0, MAX_ACTUAL_PAGE_COUNT, PMM_STATE_RESERVED);

    Physical addr = createPhysical (0);
    kpmm_free (addr, 2);
    EQ_SCALAR (panic_invoked, true);

    END();
}

TEST (PMM, free_doubleFree)
{
    Physical addr = createPhysical (0);
    kpmm_free (addr, 2);
    EQ_SCALAR (panic_invoked, true);

    END();
}

TEST (PMM, free_success)
{
    Physical addresses[] = {
        createPhysical (0),
        createPhysical (ACTUAL_MEMORY_SIZE - 2 * CONFIG_PAGE_FRAME_SIZE_BYTES),
    };

    // Every page is used.
    set_pab (pab, 0, MAX_ACTUAL_PAGE_COUNT, PMM_STATE_USED);

    // Frees two consecutive pages.
    for (int i = 0; i < ARRAY_LENGTH (addresses); i++)
    {
        Physical addr = addresses[i];
        printf ("\n:: Freeing x:%x, %x", addr.val, addr.val + CONFIG_PAGE_FRAME_SIZE_BYTES);
        EQ_SCALAR (true, kpmm_free (addr, 2));

        validate_pab (pab, addr.val, PMM_STATE_FREE);
        validate_pab (pab, addr.val + CONFIG_PAGE_FRAME_SIZE_BYTES, PMM_STATE_FREE);
    }

    END();
}

TEST (PMM, memSize_zerofree)
{
    // Set every page in PAB.
    set_pab (pab, 0, MAX_ACTUAL_PAGE_COUNT, PMM_STATE_USED);
    size_t freeMemSize = kpmm_getFreeMemorySize();
    EQ_SCALAR (freeMemSize, 0);

    END();
}

TEST (PMM, memSize_somefree)
{
    // Set every page in PAB.
    set_pab (pab, 0, MAX_ACTUAL_PAGE_COUNT, PMM_STATE_USED);

    // Free last 2 pages
    USYSINT lastPageAddress = (MAX_ACTUAL_PAGE_COUNT - 2) * CONFIG_PAGE_FRAME_SIZE_BYTES;
    Physical startAddress   = createPhysical (lastPageAddress);
    EQ_SCALAR (true, kpmm_free (startAddress, 2));

    size_t freeMemSize = kpmm_getFreeMemorySize();
    EQ_SCALAR (freeMemSize, 4096 * 2);

    END();
}

UINT powerOfTwo (UINT e) { return pow (2, e); }

static void set_pab (U8 *const pab, USYSINT start, UINT pgCount, KernelPhysicalMemoryStates state)
{
    for (; pgCount > 0; pgCount--, start += CONFIG_PAGE_FRAME_SIZE_BYTES)
    {
        USYSINT pageIndex = start / CONFIG_PAGE_FRAME_SIZE_BYTES;
        UINT byte         = PAB_BYTE (pageIndex);
        UINT bit          = PAB_BIT (pageIndex);

        pab[byte] &= ~(MAX_STATE << bit);
        pab[byte] |= (state << bit);
    }
}

static void validate_pab (const U8 *pab, USYSINT addr, KernelPhysicalMemoryStates state)
{
    USYSINT pageIndex = addr / CONFIG_PAGE_FRAME_SIZE_BYTES;
    UINT byte         = PAB_BYTE (pageIndex);
    UINT bit          = PAB_BIT (pageIndex);

    EQ_SCALAR (state, (pab[byte] & (1 << bit)) >> bit);
}

void reset()
{
    panic_invoked = false;
    k_errorNumber = ERR_NONE;
    resetX86Pmm();
    resetUtilsFake();

    power_of_two_fake.handler = powerOfTwo;

    // Default size of RAM is set to 2 MB.
    kpmm_arch_getInstalledMemoryByteCount_fake.ret = 2 * MB;

    init_pab();
}

int main()
{
#ifdef DEBUG
    // Cannot test asserts in non DEBUG build.
    pab_uninitialized_mustfail();
#endif

    // Will just set pointer `s_pab` to point to `pab` buffer defined here. No allocation or
    // deallocation will take place as memory map count and files count are not set and will be at
    // default zero.
    kpmm_init();

    zero_page_count();
    allocat_success();
    alloc_success();
    alloc_allocAt_outOfMem();
    free_success();
    free_allocat_misaligned();
    free_allocat_outsideRange();
    // TODO: Why is double free panics but not double alloc?
    free_doubleFree();
    free_reservePages();
    memSize_zerofree();
    memSize_somefree();
}
