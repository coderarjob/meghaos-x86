/*
 * --------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Kernel and usermode paging
 * structures and casting methods.
 *
 * Methods to create PD, PT, map/unmap physical and virtual pages etc, in a Cross
 * platform way.
 * --------------------------------------------------------------------------------
 */

#ifndef PAGING_H
#define PAGING_H

#include <types.h>
#include <stdbool.h>
#include <config.h>
#include <utils.h>

typedef struct PageAttributes
{
    bool present;
    bool writable;
    bool userAccessable;
    bool cacheDisable;
    bool writeThrough;
} __attribute__((aligned(4))) PageAttributes;

typedef struct ArchPageDirectoryEntry ArchPageDirectoryEntry;
typedef struct ArchPageTableEntry ArchPageTableEntry;

typedef ArchPageDirectoryEntry* PageDirectory;
typedef ArchPageTableEntry* PageTable;

typedef enum PagingMapFlags
{
    PG_MAP_KERNEL        = (1 << 0),
    PG_MAP_CACHE_ENABLED = (1 << 1),
    PG_MAP_WRITABLE      = (1 << 2),
} PagingMapFlags;

// Physical start of the page frame 'pf'.
#define PAGEFRAME_TO_PHYSICAL(pf) ((USYSINT)(pf) << CONFIG_PAGE_SIZE_BITS)

// Page frame from the physical start address of the page frame.
// Assumption is 'addr' to always be page aligned.
#define PHYSICAL_TO_PAGEFRAME(addr)                                                                \
    ((addr / CONFIG_PAGE_FRAME_SIZE_BYTES) & BIT_MASK (CONFIG_PAGE_FRAME_SIZE_BITS, 0))

// Number of complete pages from at-most 'b' number of bytes.
#define BYTES_TO_PAGEFRAMES_FLOOR(b)  (PHYSICAL_TO_PAGEFRAME(b))

// Number of complete pages from at-least 'b' number of bytes.
#define BYTES_TO_PAGEFRAMES_CEILING(b)  \
    (PHYSICAL_TO_PAGEFRAME(ALIGN_UP ((b), CONFIG_PAGE_FRAME_SIZE_BYTES)))

PageDirectory kpg_getcurrentpd();
void kpg_setupPD (PageDirectory pd, PageAttributes *attr);
void kpg_setPT (PageDirectory pd, PageTable *pt, PTR start);
bool kpg_map (PageDirectory pd, PTR va, Physical pa, PagingMapFlags flags);
bool kpg_unmap (PageDirectory pd, PTR va);
void* kpg_temporaryMap (Physical pa);
void kpg_temporaryUnmap();

#endif // PAGING_H
