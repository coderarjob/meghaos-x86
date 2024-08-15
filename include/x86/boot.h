/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Bootloader
 *
 * Contains abstract structure declarations and methods that is used to get information from the
 * bootloader.
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#include <buildcheck.h>

enum BootMemoryMapTypes {
    MMTYPE_FREE         = 1,
    MMTYPE_RESERVED     = 2,
    MMTYPE_ACPI_RECLAIM = 3,
};

typedef U32 BootMemoryMapTypes;
typedef struct BootLoaderInfo BootLoaderInfo;

typedef struct BootMemoryMapItem {
    const U64 baseAddr;
    const U64 length;
    const BootMemoryMapTypes type;
} __attribute__ ((packed)) BootMemoryMapItem;

typedef struct BootFileItem {
    const U32 startLocation;
    const U16 length;
} __attribute__ ((packed)) BootFileItem;

typedef struct GraphisModeInfo {
    const U16 xResolution;
    const U16 yResolution;
    const U8 bitsPerPixel;
    const U16 graphicsMode;
    const U16 vbeVersion;
    const Physical framebufferPhysicalPtr;
    const U16 bytesPerScanLine;
} __attribute__ ((packed)) GraphisModeInfo;

U16 kboot_getBootFileItemCount();
BootFileItem kboot_getBootFileItem (INT index);
BootMemoryMapItem kboot_getBootMemoryMapItem (INT index);
U16 kboot_getBootMemoryMapItemCount();
ULLONG kboot_calculateInstalledMemory();

GraphisModeInfo kboot_getGraphicsModeInfo();
