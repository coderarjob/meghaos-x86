#include <mock/common/bitmap.h>
#include <unittest/fake.h>

DEFINE_FUNC_2(BitmapState, bitmap_get, Bitmap*, UINT);
DEFINE_FUNC_4(bool, bitmap_setContinous, Bitmap *, UINT, UINT, BitmapState);
DEFINE_FUNC_3(INT, bitmap_findContinous, Bitmap *, BitmapState, UINT);
DEFINE_FUNC_4(bool, bitmap_findContinousAt, Bitmap *, BitmapState, UINT, UINT);

void resetBitmapFake()
{
    RESET_FAKE(bitmap_get);
    RESET_FAKE(bitmap_setContinous);
    RESET_FAKE(bitmap_findContinous);
    RESET_FAKE(bitmap_findContinousAt);
}
