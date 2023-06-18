#ifndef BITMAP_FAKE_H
#define BITMAP_FAKE_H

#include <unittest/fake.h>
#include <common/bitmap.h>

DECLARE_FUNC(BitmapState, bitmap_get, Bitmap *, UINT);
DECLARE_FUNC(bool, bitmap_setContinous, Bitmap *, UINT, UINT, BitmapState);
DECLARE_FUNC(INT, bitmap_findContinous, Bitmap *, BitmapState, UINT);
DECLARE_FUNC(bool, bitmap_findContinousAt, Bitmap *, BitmapState, UINT, UINT);

void resetBitmapFake();
#endif // BITMAP_FAKE_H

