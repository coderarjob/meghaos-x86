#include <mock/kernel/bitmap.h>
#include <unittest/fake.h>

DEFINE_FUNC(BitmapState, bitmap_get, Bitmap*, UINT);
DEFINE_FUNC(bool, bitmap_setContinous, Bitmap *, UINT, UINT, BitmapState);
DEFINE_FUNC(INT, bitmap_findContinous, Bitmap *, BitmapState, UINT);
DEFINE_FUNC(bool, bitmap_findContinousAt, Bitmap *, BitmapState, UINT, UINT);

void resetBitmapFake()
{
    RESET_FAKE(bitmap_get);
    RESET_FAKE(bitmap_setContinous);
    RESET_FAKE(bitmap_findContinous);
    RESET_FAKE(bitmap_findContinousAt);
}
