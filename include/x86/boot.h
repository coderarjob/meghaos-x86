/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Bootloader
 *
 * Contains abstract structure declarations and methods that is used to get information from the
 * bootloader.
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#include <config.h>
#include <types.h>
#include <buildcheck.h>

#define BOOT_FONTS_GLYPH_COUNT 128U
#define BOOT_FONTS_GLYPH_BYTES ((CONFIG_GXMODE_FONT_WIDTH * CONFIG_GXMODE_FONT_HEIGHT) / 8U)

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
    const U8 name[CONFIG_BOOT_FILENAME_LEN_CHARS];
    const U32 startLocation;
    const U16 length;
} __attribute__ ((packed)) BootFileItem;

typedef struct BootGraphicsModeInfo {
    const U16 xResolution;
    const U16 yResolution;
    const U8 bitsPerPixel;
    const U16 graphicsMode;
    const U16 vbeVersion;
    const Physical framebufferPhysicalPtr;
    const U16 bytesPerScanLine;
} __attribute__ ((packed)) BootGraphicsModeInfo;

U16 kboot_getBootFileItemCount();
BootFileItem kboot_getBootFileItem (INT index);
BootMemoryMapItem kboot_getBootMemoryMapItem (INT index);
U16 kboot_getBootMemoryMapItemCount();
ULLONG kboot_calculateInstalledMemory();

BootGraphicsModeInfo kboot_getGraphicsModeInfo();
const U8* kboot_getFontData();
