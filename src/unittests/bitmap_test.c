#include <common/bitmap.h>
#include <stdio.h>
#include <string.h>
#include <types.h>
#include <unittest/unittest.h>
#include <utils.h>
#include <kerror.h>
#include <unittest/fake.h>
#include <stdarg.h>

DECLARE_FUNC(bool, isValid, UINT, BitmapState, BitmapState);
DEFINE_FUNC_3(bool, isValid, UINT, BitmapState, BitmapState);

static BitmapState bitmap[3];
static Bitmap b = {bitmap, sizeof(bitmap), 2, isValid};

typedef enum States
{
    STATE_1,
    STATE_2,
    STATE_3,
    STATE_4,
    STATE_5
} States;

/* Fake Defination. At present, meghatest does not support varidac parameters. */
static bool panic_invoked;
void k_panic_ndu (const CHAR *s,...)
{
    panic_invoked = true;

    va_list l;
    va_start (l, s);

    printf ("%s", COL_RED);
    vprintf (s, l);
    printf ("%s", COL_RESET);

    va_end(l);
}

/**************************************************************************************************
 * Bitmap bits per state must be a factor of 8.
 * 3 is not a factor of 8, so must hit assert.
**************************************************************************************************/
TEST(bitmap, bitmap_invalid_bitmap_mustfail) {
  Bitmap b = {bitmap, sizeof(bitmap), 3, isValid};

  bitmap_setContinous(&b, 0, 1, 1);
  EQ_SCALAR(panic_invoked, true);

  END();
}

/**************************************************************************************************
 * Sets invalid set to bitmap.
**************************************************************************************************/
TEST(bitmap, set_invalid_state_mustfail) {
  isValid_fake.ret = true;

  bitmap_setContinous(&b, 0, 1, STATE_5);
  EQ_SCALAR(panic_invoked, true);

  END();
}

/**************************************************************************************************
 * No change must be make to bitmap if 'allow` function returns false.
**************************************************************************************************/
TEST(bitmap, set_not_allowed_mustfail) {
  isValid_fake.ret = false;

  INT ret = bitmap_setContinous(&b, 5, 1, 1);
  EQ_SCALAR(ret, KERNEL_EXIT_FAILURE);

  U8 final[] = {0xFF,0xFF,0xFF};
  EQ_MEM(bitmap,final, sizeof(bitmap));
  END();
}

/**************************************************************************************************
 * Invalid state index. Index > Count
**************************************************************************************************/
TEST(bitmap, set_large_index_mustfail) {
  isValid_fake.ret = true;

  bitmap_setContinous(&b, BITMAP_ITEM_COUNT(&b), 1, STATE_1);
  EQ_SCALAR(panic_invoked, true);

  END();
}

/**************************************************************************************************
 * Sets all valid states to bitmap
**************************************************************************************************/
TEST(bitmap, set_valid_states_mustpass) {
  isValid_fake.ret = true;

  EQ_SCALAR(bitmap_setContinous(&b, 0, 1, STATE_1), KERNEL_EXIT_SUCCESS);
  EQ_SCALAR(bitmap_setContinous(&b, 1, 1, STATE_2), KERNEL_EXIT_SUCCESS);
  EQ_SCALAR(bitmap_setContinous(&b, 2, 1, STATE_3), KERNEL_EXIT_SUCCESS);
  EQ_SCALAR(bitmap_setContinous(&b, 3, 1, STATE_4), KERNEL_EXIT_SUCCESS);

  U8 final[] = {0xE4,0xFF,0xFF};
  EQ_MEM(bitmap,final, sizeof(bitmap));
  END();
}


/**************************************************************************************************
 * Sets one state in the bitmap.
**************************************************************************************************/
TEST(bitmap, set_one_mustpass) {
  isValid_fake.ret = true;

  INT ret = bitmap_setContinous(&b, 5, 1, 1);
  EQ_SCALAR(ret, KERNEL_EXIT_SUCCESS);

  U8 final[] = {0xFF,0xF7,0xFF};
  EQ_MEM(bitmap,final, sizeof(bitmap));
  END();
}

/**************************************************************************************************
 * Sets 5 consecutive states in the bitmap.
**************************************************************************************************/
TEST(bitmap, set_three_mustpass) {
  isValid_fake.ret = true;

  INT ret = bitmap_setContinous(&b, 0, 5, 1);
  EQ_SCALAR(ret, KERNEL_EXIT_SUCCESS);

  U8 final[] = {0x55,0xFD,0xFF};
  EQ_MEM(bitmap,final, sizeof(bitmap));
  END();
}

/**************************************************************************************************
 * Sets every state in the bitmap.
**************************************************************************************************/
TEST(bitmap, set_all_mustpass) {
  isValid_fake.ret = true;

  INT ret = bitmap_setContinous(&b, 0, BITMAP_ITEM_COUNT(&b), 1);
  EQ_SCALAR(ret, KERNEL_EXIT_SUCCESS);

  U8 final[] = {0x55,0x55,0x55};
  EQ_MEM(bitmap,final, sizeof(bitmap));
  END();
}

/**************************************************************************************************
 * Sets the very last state in the bitmap.
**************************************************************************************************/
TEST(bitmap, set_laststate_mustpass) {
  isValid_fake.ret = true;

  INT ret = bitmap_setContinous(&b, BITMAP_ITEM_COUNT(&b) - 1, 1, 1);
  EQ_SCALAR(ret, KERNEL_EXIT_SUCCESS);

  U8 final[] = {0xFF,0xFF,0x7F};
  EQ_MEM(bitmap,final, sizeof(bitmap));
  END();
}

/**************************************************************************************************
 * Single state is flipped, to tests change if it effects any other state.
**************************************************************************************************/
TEST(bitmap, set_multiple_mustpass) {
  isValid_fake.ret = true;

  EQ_SCALAR(KERNEL_EXIT_SUCCESS, bitmap_setContinous(&b, 5, 1, 1));
  EQ_SCALAR(KERNEL_EXIT_SUCCESS, bitmap_setContinous(&b, 6, 1, 2));
  EQ_SCALAR(KERNEL_EXIT_SUCCESS, bitmap_setContinous(&b, 6, 1, 1));

  U8 final[] = {0xFF,0xD7,0xFF};
  EQ_MEM(bitmap, final, sizeof(bitmap));
  END();
}

/*TEST(bitmap, get_mustpass) {
  memset(bitmap, 0xFF, ARRAY_LENGTH(bitmap));
  Bitmap b = {bitmap, ARRAY_LENGTH(bitmap)};

  bitmap_set(&b, 5, 1);

  BitmapStates states[3];
  states[0] = bitmap_get(&b, 4);
  states[1] = bitmap_get(&b, 5);
  states[2] = bitmap_get(&b, 6);

  BitmapStates final[] = {3, 1, 3};
  EQ_MEM(states, final, sizeof(states));
  END();
}*/

/**************************************************************************************************
 * Finds continous states from bitmap.
**************************************************************************************************/
TEST(bitmap, findContinous_mustpass) {
    isValid_fake.ret = true;

    bitmap_setContinous(&b, 3, 7, 1);

    U8 starting[] = {0x7F,0x55,0xF5};
    EQ_MEM(bitmap, starting, sizeof(bitmap));

    EQ_SCALAR(3, bitmap_findContinous(&b, 1, 1));
    EQ_SCALAR(KERNEL_EXIT_SUCCESS,bitmap_setContinous(&b, 3, 1, 3));

    EQ_SCALAR(4, bitmap_findContinous(&b, 1, 3));
    EQ_SCALAR(KERNEL_EXIT_SUCCESS,bitmap_setContinous(&b, 4, 3, 3));

    EQ_SCALAR(7, bitmap_findContinous(&b, 1, 3));
    EQ_SCALAR(KERNEL_EXIT_SUCCESS,bitmap_setContinous(&b, 7, 3, 3));

    EQ_SCALAR(KERNEL_EXIT_FAILURE, bitmap_findContinous(&b, 1, 1));

    U8 final[] = {0xFF,0xFF,0xFF};
    EQ_MEM(bitmap, final, sizeof(bitmap));

    END();
}

void reset() {
    memset(bitmap, 0xFF, ARRAY_LENGTH(bitmap));
    panic_invoked = false;
}

int main() {
    bitmap_invalid_bitmap_mustfail();
    set_large_index_mustfail();
    set_not_allowed_mustfail();
    set_invalid_state_mustfail();
    set_valid_states_mustpass();
    set_all_mustpass();
    set_one_mustpass();
    set_three_mustpass();
    set_laststate_mustpass();
    set_multiple_mustpass();
    findContinous_mustpass();
}
