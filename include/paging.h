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

typedef struct PageDirectory
{
    PageAttributes attr;
    ArchPageDirectoryEntry *pd;
} PageDirectory;

typedef struct PageTable
{
    PageAttributes attr;
    ArchPageTableEntry *pt;
} PageTable;

typedef enum PageMapAttributes
{
    PAGE_MAP_UNMAPED,
    PAGE_MAP_BACKED,
    PAGE_MAP_UNBACKED
} PageMapAttributes;

PageDirectory kpg_getcurrentpd();
void kpg_setupPD (PageDirectory *pd, PageAttributes *attr);
void kpg_setPT (PageDirectory *pd, PageTable *pt, PTR start);
bool kpg_map (PageDirectory *pd, PageMapAttributes attr, PTR va, SIZE sz, Physical *pa);

#endif // PAGING_H
