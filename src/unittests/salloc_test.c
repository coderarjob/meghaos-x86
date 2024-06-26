#include <utils.h>
#include <string.h>
#include <unittest/unittest.h>
#include <mock/kernel/x86/memmanage.h>
#include <types.h>
#include <memmanage.h>
#include <kerror.h>
#include <x86/kernel.h>

static U8 salloc_buffer[SALLOC_SIZE_BYTES];

#define ALIGNED_SIZE(sz) ALIGN_UP ((sz), SALLOC_GRANUALITY)

/*
 * | TEST CASES                                              | TEST FUNCTION                 |
 * |---------------------------------------------------------|-------------------------------|
 * | salloc/scalloc  - Given a valid input. Success          | small_allocations_success,    |
 * |                                                         | large_allocations_success     |
 * | salloc/scalloc  - Invalid input. Out of range error.    | invalid_inputs                |
 * | salloc          - Given a valid which exceeds the       |                               |
 * |                   available space. Out of memory error. | salloc_out_of_memory          |
 * | salloc          - Alignment of allocations.             | salloc_alignments             |
 * |---------------------------------------------------------|-------------------------------|
 * */

TEST (salloc, small_allocations_success)
{
    EQ_SCALAR ((PTR)salloc (SALLOC_GRANUALITY), (PTR)salloc_buffer);
    EQ_SCALAR ((PTR)scalloc (SALLOC_GRANUALITY), (PTR)salloc_buffer + SALLOC_GRANUALITY);
    END();
}

TEST (salloc, large_allocations_success)
{
    EQ_SCALAR ((PTR)salloc (SALLOC_SIZE_BYTES), (PTR)salloc_buffer);
    EQ_SCALAR ((PTR)scalloc (1), (PTR)NULL);
    END();
}

TEST (salloc, salloc_out_of_memory)
{
    EQ_SCALAR ((PTR)salloc (SALLOC_SIZE_BYTES), (PTR)salloc_buffer);
    EQ_SCALAR ((PTR)salloc (1), (PTR)NULL);
    EQ_SCALAR (g_kstate.errorNumber, ERR_OUT_OF_MEM);
    END();
}

TEST (salloc, salloc_alignments)
{
    PTR ret1 = (PTR)salloc (1);
    PTR ret2 = (PTR)salloc (SALLOC_GRANUALITY);
    PTR ret3 = (PTR)salloc (1);

    EQ_SCALAR (ret1, (PTR)salloc_buffer);
    EQ_SCALAR (ret2, (PTR)salloc_buffer + ALIGNED_SIZE (1));
    EQ_SCALAR (ret3, (PTR)salloc_buffer + ALIGNED_SIZE (1) + ALIGNED_SIZE (SALLOC_GRANUALITY));
    END();
}

TEST (salloc, invalid_inputs)
{
    SIZE invalid_inputs[] = { 0, SALLOC_SIZE_BYTES + 1 };

    for (int i = 0; i < 2; i++) {
        EQ_SCALAR ((PTR)scalloc (invalid_inputs[i]), 0);
        EQ_SCALAR (g_kstate.errorNumber, ERR_INVALID_RANGE);
        g_kstate.errorNumber = ERR_NONE;

        EQ_SCALAR ((PTR)salloc (invalid_inputs[i]), 0);
        EQ_SCALAR (g_kstate.errorNumber, ERR_INVALID_RANGE);
        g_kstate.errorNumber = ERR_NONE;
    }

    END();
}

TEST (salloc, get_used_memory)
{
    // When there are no allocation
    EQ_SCALAR (salloc_getUsedMemory(), 0U);

    // When some memory is used.
    SIZE sizes[] = { ALIGNED_SIZE (SALLOC_SIZE_BYTES / 2), ALIGNED_SIZE (SALLOC_SIZE_BYTES / 3) };
    NEQ_SCALAR (salloc (sizes[0]), NULL);
    NEQ_SCALAR (salloc (sizes[1]), NULL);

    EQ_SCALAR (salloc_getUsedMemory(), (sizes[0] + sizes[1]));
    END();
}

void reset()
{
    resetX86MemManageFake();

    salloc_arch_preAllocateMemory_fake.ret = salloc_buffer;
    g_kstate.errorNumber                   = ERR_NONE;

    salloc_init();
}

int main()
{
    small_allocations_success();
    large_allocations_success();
    invalid_inputs();
    salloc_alignments();
    salloc_out_of_memory();
    get_used_memory();
}
