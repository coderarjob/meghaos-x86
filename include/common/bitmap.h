/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Common - Bitmap
*
* Bitmap implementation for use in the OS.
* ---------------------------------------------------------------------------
*/

#ifndef COMMON_BITMAP_H
#define COMMON_BITMAP_H

#include <types.h>
#include <assert.h>
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

static inline SIZE bitmap_bitsPerState(Bitmap *b)
{
    k_assert(8 % b->bitsPerState == 0, "Must be a factor of 8.");
    return b->bitsPerState;
}

#define BITMAP_STATE_MASK(b)      (power_of_two((b)->bitsPerState) - 1)
#define BITMAP_STATES_PER_BYTE(b) (8 / bitmap_bitsPerState(b))
#define BITMAP_ITEM_COUNT(b)      ((b)->size * BITMAP_STATES_PER_BYTE(b))

INT bitmap_get (Bitmap *b, UINT index);
INT bitmap_setContinous (Bitmap *b, UINT index, UINT len, BitmapState state);
INT bitmap_findContinous (Bitmap *b, BitmapState state, UINT len);
INT bitmap_findContinousAt (Bitmap *b, BitmapState state, UINT len, UINT indexAt);
#endif // COMMON_BITMAP_H
