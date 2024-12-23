/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Bitmap
*
* Bitmap implementation for use in the OS.
* ---------------------------------------------------------------------------
*/

#pragma once

#include <types.h>
#include <utils.h>

typedef U8 BitmapState;

typedef struct Bitmap
{
    BitmapState *bitmap;           /* Pointer to the actual Bitmap */
    SIZE size;                     /* Number of bytes in the bitmap array. */
    SIZE bitsPerState;             /* one state in bitmap requires this many bits. Must be a factor
                                      of 8. */
    bool (*allow)(UINT index,
                 BitmapState old,
                 BitmapState new); /* Function returns true if state change can be done. */
} Bitmap;

#define BITMAP_MAX_STATE(b)       (POWER_OF_TWO((b)->bitsPerState))
#define BITMAP_STATE_MASK(b)      (POWER_OF_TWO((b)->bitsPerState) - 1)
#define BITMAP_STATES_PER_BYTE(b) (8 / ((b)->bitsPerState))
#define BITMAP_CAPACITY(b)        ((b)->size * BITMAP_STATES_PER_BYTE(b))

BitmapState bitmap_get (Bitmap *b, UINT index);
bool bitmap_setContinous (Bitmap *b, UINT index, UINT len, BitmapState state);
INT bitmap_findContinous (Bitmap *b, BitmapState state, UINT len);
bool bitmap_findContinousAt (Bitmap *b, BitmapState state, UINT len, UINT indexAt);
