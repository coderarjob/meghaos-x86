#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <mock/kernel/bitmap.h>

DEFINE_FUNC(BitmapState, bitmap_get, Bitmap*, UINT);
DEFINE_FUNC(bool, bitmap_setContinous, Bitmap *, UINT, UINT, BitmapState);
DEFINE_FUNC(INT, bitmap_findContinous, Bitmap *, BitmapState, UINT);
DEFINE_FUNC(bool, bitmap_findContinousAt, Bitmap *, BitmapState, UINT, UINT);

void resetBitmapFake()
{
    RESET_MOCK(bitmap_get);
    RESET_MOCK(bitmap_setContinous);
    RESET_MOCK(bitmap_findContinous);
    RESET_MOCK(bitmap_findContinousAt);
}
