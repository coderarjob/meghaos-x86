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
    PG_MAP_FLAG_KERNEL        = (1 << 0),
    PG_MAP_FLAG_CACHE_ENABLED = (1 << 1),
    PG_MAP_FLAG_WRITABLE      = (1 << 2),
    PG_MAP_FLAG_NOT_PRESENT   = (1 << 3),
} PagingMapFlags;

// Physical start of the page frame 'pf'.
#define PAGEFRAME_TO_PHYSICAL(pf) (PAGEFRAMES_TO_BYTES(pf))

// A linear address to corresponding number of page frames.
#define PHYSICAL_TO_PAGEFRAME(addr) (BYTES_TO_PAGEFRAMES_FLOOR(addr))

PageDirectory kpg_getcurrentpd();
//void kpg_setupPD (PageDirectory pd, PageAttributes *attr);
//void kpg_setPT (PageDirectory pd, PageTable *pt, PTR start);
bool kpg_map (PageDirectory pd, PTR va, Physical pa, PagingMapFlags flags);
bool kpg_unmap (PageDirectory pd, PTR va);
void* kpg_temporaryMap (Physical pa);
void kpg_temporaryUnmap();
bool kpg_getPhysicalMapping (PageDirectory pd, PTR va, Physical* pa);

#endif // PAGING_H
