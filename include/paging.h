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

typedef struct PageAttributes {
    bool present;
    bool writable;
    bool userAccessable;
    bool cacheDisable;
    bool writeThrough;
} __attribute__ ((aligned (4))) PageAttributes;

typedef struct ArchPageDirectoryEntry ArchPageDirectoryEntry;
typedef struct ArchPageTableEntry ArchPageTableEntry;

typedef ArchPageDirectoryEntry* PageDirectory;
typedef ArchPageTableEntry* PageTable;

typedef enum PagingMapFlags {
    PG_MAP_FLAG_KERNEL         = (1 << 0),
    PG_MAP_FLAG_CACHE_ENABLED  = (1 << 1),
    PG_MAP_FLAG_WRITABLE       = (1 << 2),
    PG_MAP_FLAG_NOT_PRESENT    = (1 << 3),
    PG_MAP_FLAG_KERNEL_DEFAULT = (PG_MAP_FLAG_KERNEL | PG_MAP_FLAG_WRITABLE |
                                  PG_MAP_FLAG_CACHE_ENABLED),
} PagingMapFlags;

typedef enum PagingOperationFlags {
    PG_NEWPD_FLAG_COPY_KERNEL_PAGES = (1 << 0),
    PG_NEWPD_FLAG_RECURSIVE_MAP     = (1 << 1),
    PG_DELPD_FLAG_KEEP_KERNEL_PAGES = (1 << 2)
} PagingOperationFlags;

// Physical start of the page frame 'pf'.
#define PAGEFRAME_TO_PHYSICAL(pf) (PAGEFRAMES_TO_BYTES (pf))

// A linear address to corresponding number of page frames.
#define PHYSICAL_TO_PAGEFRAME(addr) (BYTES_TO_PAGEFRAMES_FLOOR (addr))

PageDirectory kpg_getcurrentpd();
bool kpg_mapContinous (PageDirectory pd, PTR vaStart, Physical paStart, SIZE numPages,
                       PagingMapFlags flags);
bool kpg_map (PageDirectory pd, PTR va, Physical pa, PagingMapFlags flags);
bool kpg_unmapContinous (PageDirectory pd, PTR vaStart, SIZE numPages);
bool kpg_unmap (PageDirectory pd, PTR va);
void* kpg_temporaryMap (Physical pa);
void kpg_temporaryUnmap();
bool kpg_doesMappingExists (PageDirectory pd, PTR va, Physical* pa);
bool kpg_createNewPageDirectory (Physical* newPD, PagingOperationFlags flags);
bool kpg_deletePageDirectory(Physical pd, PagingOperationFlags flags);

#endif // PAGING_H
