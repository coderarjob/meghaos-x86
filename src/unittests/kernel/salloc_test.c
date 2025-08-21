#define YUKTI_TEST_STRIP_PREFIX
#define YUKTI_TEST_IMPLEMENTATION
#include <unittest/yukti.h>
#include <stdint.h>
#include <utils.h>
#include <string.h>
#include <types.h>
#include <memmanage.h>
#include <kerror.h>
#include <kernel.h>
#include <mosunittest.h>

// Must be multiple of SALLOC_GRANUALITY for some tests to pass
#define UT_SALLOC_SIZE_BYTES 64

static U8 salloc_buffer[UT_SALLOC_SIZE_BYTES];

#define ALIGNED_SIZE(sz) ALIGN_UP ((sz), SALLOC_GRANUALITY)

/*
 * | TEST CASES                                              | TEST FUNCTION                 |
 * |---------------------------------------------------------|-------------------------------|
 * | ksalloc/kscalloc - Given a valid input. Success         | small_allocations_success,    |
 * |                                                         | large_allocations_success     |
 * | ksalloc/kscalloc - Invalid input. Out of range error.   | invalid_inputs                |
 * | ksalloc          - Given a valid which exceeds the      |                               |
 * |                    available space. Out of memory error.| salloc_out_of_memory          |
 * | ksalloc          - Alignment of allocations.            | salloc_alignments             |
 * |---------------------------------------------------------|-------------------------------|
 * */

TEST (ksalloc, small_allocations_success)
{
    EQ_SCALAR ((PTR)ksalloc (SALLOC_GRANUALITY), (PTR)salloc_buffer);
    EQ_SCALAR ((PTR)kscalloc (SALLOC_GRANUALITY), (PTR)salloc_buffer + SALLOC_GRANUALITY);
    END();
}

TEST (ksalloc, large_allocations_success)
{
    EQ_SCALAR ((PTR)ksalloc (UT_SALLOC_SIZE_BYTES), (PTR)salloc_buffer);
    EQ_SCALAR ((PTR)kscalloc (1), (PTR)NULL);
    END();
}

TEST (ksalloc, salloc_out_of_memory)
{
    EQ_SCALAR ((PTR)ksalloc (UT_SALLOC_SIZE_BYTES), (PTR)salloc_buffer);
    EQ_SCALAR ((PTR)ksalloc (1), (PTR)NULL);
    EQ_SCALAR (g_kstate.errorNumber, (UINT)ERR_OUT_OF_MEM);
    END();
}

TEST (ksalloc, salloc_alignments)
{
    PTR ret1 = (PTR)ksalloc (1);
    PTR ret2 = (PTR)ksalloc (SALLOC_GRANUALITY);
    PTR ret3 = (PTR)ksalloc (1);

    EQ_SCALAR (ret1, (PTR)salloc_buffer);
    EQ_SCALAR (ret2, (PTR)salloc_buffer + ALIGNED_SIZE (1));
    EQ_SCALAR (ret3, (PTR)salloc_buffer + ALIGNED_SIZE (1) + ALIGNED_SIZE (SALLOC_GRANUALITY));
    END();
}

TEST (ksalloc, invalid_inputs)
{
    SIZE invalid_inputs[] = { 0, UT_SALLOC_SIZE_BYTES + 1 };

    for (int i = 0; i < 2; i++) {
        EQ_SCALAR ((PTR)kscalloc (invalid_inputs[i]), 0U);
        EQ_SCALAR (g_kstate.errorNumber, (UINT)ERR_INVALID_RANGE);
        g_kstate.errorNumber = ERR_NONE;

        EQ_SCALAR ((PTR)ksalloc (invalid_inputs[i]), 0U);
        EQ_SCALAR (g_kstate.errorNumber, (UINT)ERR_INVALID_RANGE);
        g_kstate.errorNumber = ERR_NONE;
    }

    END();
}

TEST (ksalloc, get_used_memory)
{
    // When there are no allocation
    EQ_SCALAR (ksalloc_getUsedMemory(), 0U);

    // When some memory is used.
    SIZE sizes[] = { ALIGNED_SIZE (UT_SALLOC_SIZE_BYTES / 2),
                     ALIGNED_SIZE (UT_SALLOC_SIZE_BYTES / 3) };
    NEQ_SCALAR ((PTR)ksalloc (sizes[0]), (PTR)NULL);
    NEQ_SCALAR ((PTR)ksalloc (sizes[1]), (PTR)NULL);

    EQ_SCALAR (ksalloc_getUsedMemory(), (sizes[0] + sizes[1]));
    END();
}

void yt_reset()
{
    g_kstate.errorNumber = ERR_NONE;

    g_utmm.arch_mem_len_bytes_salloc = UT_SALLOC_SIZE_BYTES;
    g_utmm.arch_mem_start_salloc     = (uintptr_t)salloc_buffer;
    ksalloc_init();
}

int main()
{
    YT_INIT();
    small_allocations_success();
    large_allocations_success();
    invalid_inputs();
    salloc_alignments();
    salloc_out_of_memory();
    get_used_memory();
    RETURN_WITH_REPORT();
}
