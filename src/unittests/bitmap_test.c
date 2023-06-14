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

/*
                         TEST CASES
 ===================================================================================================
Common
- bitsPerState not multiple of 8  | panics | bitmap_bitsPerState_invalid_mustfail
- b == NULL                       | panics | bitmap_null_mustfail

bitmap_get
- index >= BITMAP_CAPACITY(b) | panics            | set_large_index_mustfail
- Read evey kind of state     | States read. true | get_mustpass

bitmap_setContinous
- state >= BITMAP_MAX_STATE(b)          | panics                  | set_invalid_state_mustfail
- b->allow == NULL                      | panics                  | set_allow_null_mustfail
- index + len - 1 >= BITMAP_CAPACITY(b) | panics                  | set_large_index_mustfail
- len == 0                              | panics                  | zero_length_mustfail
- allow returns false                   | Bitmap inchanged. false | set_not_allowed_mustfail
- allow returns true                    | Changes Bitmap. true    | any positive test case
- Write evey kind of state              | States written. true    | set_valid_states_mustpass
- Fill bitmap with some state,          | Changes bitmap. true    | set_all_mustpass
- Setting one state effects no other    | Changes bitmap. true.   | set_multiple_mustpass
- Setting the very last state           | Changes bitmap. true.   | set_laststate_mustpass

bitmap_findContinous
- state >= BITMAP_MAX_STATE(b)  | panics                      | set_invalid_state_mustfail
- len == 0                      | panics                      | zero_length_mustfail
- Continous block is available  | Finds correct block. true   | findContinous_mustpass
- Continous block is unavilable | false                       | findContinous_mustpass

bitmap_findContinousAt
- state >= BITMAP_MAX_STATE(b)          | panics                    | set_invalid_state_mustfail
- index + len - 1 >= BITMAP_CAPACITY(b) | panics                    | set_large_index_mustfail
- len == 0                              | panics                    | zero_length_mustfail
- Continous block is available          | Finds correct block. true | findContinousAt_mustpass
- Continous block is unavilable         | false                     | findContinousAt_mustpass
 */
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

    printf ("%s!%s", COL_RED, COL_RESET);

    va_end(l);
}

/**************************************************************************************************
 * Bitmap bits per state must be a factor of 8.
 * 3 is not a factor of 8, so must hit assert.
**************************************************************************************************/
TEST(bitmap, bitmap_bitsPerState_invalid_mustfail) {
  Bitmap b = {bitmap, sizeof(bitmap), 3, isValid};

  bitmap_setContinous(&b, 0, 1, 1);
  EQ_SCALAR(panic_invoked, true);

  bitmap_get(&b, 0);
  EQ_SCALAR(panic_invoked, true);

  bitmap_findContinous(&b, STATE_1, 1);
  EQ_SCALAR(panic_invoked, true);

  bitmap_findContinousAt(&b, STATE_1, 1, 0);
  EQ_SCALAR(panic_invoked, true);

  END();
}

/**************************************************************************************************
 * Bitmap = NULL  passed
**************************************************************************************************/
TEST(bitmap, bitmap_null_mustfail) {
  bitmap_setContinous(NULL, 0, 1, 1);
  EQ_SCALAR(panic_invoked, true);

  bitmap_get(NULL, 0);
  EQ_SCALAR(panic_invoked, true);

  bitmap_findContinous(NULL, STATE_1, 1);
  EQ_SCALAR(panic_invoked, true);

  bitmap_findContinousAt(NULL, STATE_1, 1, 0);
  EQ_SCALAR(panic_invoked, true);

  END();
}

/**************************************************************************************************
 * Sets invalid set to bitmap.
**************************************************************************************************/
TEST(bitmap, invalid_state_mustfail) {
  isValid_fake.ret = true;

  bitmap_setContinous(&b, 0, 1, STATE_5);
  EQ_SCALAR(panic_invoked, true);

  panic_invoked = false;
  bitmap_findContinous(&b, STATE_5, 1);
  EQ_SCALAR(panic_invoked, true);

  panic_invoked = false;
  bitmap_findContinousAt(&b, STATE_5, 1, 0);
  EQ_SCALAR(panic_invoked, true);

  END();
}

/**************************************************************************************************
 * Lengh = 0
**************************************************************************************************/
TEST(bitmap, zero_length_mustfail) {
  isValid_fake.ret = true;

  bitmap_setContinous(&b, 0, 0, STATE_5);
  EQ_SCALAR(panic_invoked, true);

  panic_invoked = false;
  bitmap_findContinous(&b, STATE_5, 0);
  EQ_SCALAR(panic_invoked, true);

  panic_invoked = false;
  bitmap_findContinousAt(&b, STATE_5, 0, 0);
  EQ_SCALAR(panic_invoked, true);

  END();
}

/**************************************************************************************************
 * No change must be make to bitmap if 'allow` function returns false.
**************************************************************************************************/
TEST(bitmap, set_not_allowed_mustfail) {
  isValid_fake.ret = false;

  EQ_SCALAR(false, bitmap_setContinous(&b, 5, 1, 1));

  U8 final[] = {0xFF,0xFF,0xFF};
  EQ_MEM(bitmap,final, sizeof(bitmap));
  END();
}

/**************************************************************************************************
 * Allow funcion is not set in bitmap.
**************************************************************************************************/
TEST(bitmap, set_allow_null_mustfail) {
  Bitmap b = {bitmap, sizeof(bitmap), 2, NULL};
  isValid_fake.ret = true;

  bitmap_setContinous(&b, 5, 1, 1);
  EQ_SCALAR(panic_invoked, true);

  END();
}

/**************************************************************************************************
 * Invalid state index. Index > Count
**************************************************************************************************/
TEST(bitmap, set_large_index_mustfail) {
  isValid_fake.ret = true;

  bitmap_setContinous(&b, BITMAP_CAPACITY(&b), 1, STATE_1);
  EQ_SCALAR(panic_invoked, true);

  panic_invoked = false;
  bitmap_get(&b, BITMAP_CAPACITY(&b));
  EQ_SCALAR(panic_invoked, true);

  panic_invoked = false;
  bitmap_findContinousAt(&b, STATE_1, BITMAP_CAPACITY(&b) + 1, 0);
  EQ_SCALAR(panic_invoked, true);

  END();
}

/**************************************************************************************************
 * Sets all valid states to bitmap
**************************************************************************************************/
TEST(bitmap, set_valid_states_mustpass) {
  isValid_fake.ret = true;

  EQ_SCALAR(bitmap_setContinous(&b, 0, 1, STATE_1), true);
  EQ_SCALAR(bitmap_setContinous(&b, 1, 1, STATE_2), true);
  EQ_SCALAR(bitmap_setContinous(&b, 2, 1, STATE_3), true);
  EQ_SCALAR(bitmap_setContinous(&b, 3, 1, STATE_4), true);

  U8 final[] = {0xE4,0xFF,0xFF};
  EQ_MEM(bitmap,final, sizeof(bitmap));
  END();
}

/**************************************************************************************************
 * Sets every state in the bitmap.
**************************************************************************************************/
TEST(bitmap, set_all_mustpass) {
  isValid_fake.ret = true;

  EQ_SCALAR(true, bitmap_setContinous(&b, 0, BITMAP_CAPACITY(&b), STATE_2));

  U8 final[] = {0x55,0x55,0x55};
  EQ_MEM(bitmap,final, sizeof(bitmap));
  END();
}

/**************************************************************************************************
 * Sets the very last state in the bitmap.
**************************************************************************************************/
TEST(bitmap, set_laststate_mustpass) {
  isValid_fake.ret = true;

  EQ_SCALAR(true, bitmap_setContinous(&b, BITMAP_CAPACITY(&b) - 1, 1, 1));

  U8 final[] = {0xFF,0xFF,0x7F};
  EQ_MEM(bitmap,final, sizeof(bitmap));
  END();
}

/**************************************************************************************************
 * Single state is flipped, to tests change if it effects any other state.
**************************************************************************************************/
TEST(bitmap, set_multiple_mustpass) {
  isValid_fake.ret = true;

  EQ_SCALAR(true, bitmap_setContinous(&b, 5, 1, 1));
  EQ_SCALAR(true, bitmap_setContinous(&b, 6, 1, 2));
  EQ_SCALAR(true, bitmap_setContinous(&b, 6, 1, 1));

  U8 final[] = {0xFF,0xD7,0xFF};
  EQ_MEM(bitmap, final, sizeof(bitmap));
  END();
}

/**************************************************************************************************
 * Write every kind of state and reack back
**************************************************************************************************/
TEST(bitmap, get_mustpass) {
  isValid_fake.ret = true;

  bitmap_setContinous(&b, 5, 1, STATE_1);
  bitmap_setContinous(&b, 6, 1, STATE_2);
  bitmap_setContinous(&b, 7, 1, STATE_3);
  bitmap_setContinous(&b, 8, 1, STATE_4);

  EQ_SCALAR(STATE_4, bitmap_get(&b, 4));
  EQ_SCALAR(STATE_1, bitmap_get(&b, 5));
  EQ_SCALAR(STATE_2, bitmap_get(&b, 6));
  EQ_SCALAR(STATE_3, bitmap_get(&b, 7));
  EQ_SCALAR(STATE_4, bitmap_get(&b, 8));
  EQ_SCALAR(STATE_4, bitmap_get(&b, 9));

  U8 final[] = {0xFF,0x93,0xFF};
  EQ_MEM(bitmap, final, sizeof(bitmap));
  END();
}

/**************************************************************************************************
 * Finds continous states from bitmap.
**************************************************************************************************/
TEST(bitmap, findContinous_mustpass) {
    isValid_fake.ret = true;

    bitmap_setContinous(&b, 3, 7, STATE_2);

    EQ_SCALAR(3, bitmap_findContinous(&b, STATE_2, 1));
    bitmap_setContinous(&b, 3, 1, STATE_4);

    EQ_SCALAR(4, bitmap_findContinous(&b, STATE_2, 3));
    bitmap_setContinous(&b, 4, 3, STATE_4);

    EQ_SCALAR(7, bitmap_findContinous(&b, STATE_2, 3));
    bitmap_setContinous(&b, 7, 3, STATE_4);

    EQ_SCALAR(KERNEL_EXIT_FAILURE, bitmap_findContinous(&b, STATE_2, 1));

    U8 final[] = {0xFF,0xFF,0xFF};
    EQ_MEM(bitmap, final, sizeof(bitmap));

    END();
}

/**************************************************************************************************
 * Finds continous states from bitmap.
**************************************************************************************************/
TEST(bitmap, findContinousAt_mustpass) {
    isValid_fake.ret = true;

    bitmap_setContinous(&b, 3, 7, STATE_2);

    EQ_SCALAR(false, bitmap_findContinousAt(&b, STATE_2, 1, 0));

    EQ_SCALAR(true, bitmap_findContinousAt(&b, STATE_2, 1, 3));
    bitmap_setContinous(&b, 3, 1, STATE_4);

    EQ_SCALAR(true, bitmap_findContinousAt(&b, STATE_2, 3, 4));
    bitmap_setContinous(&b, 4, 3, STATE_4);

    EQ_SCALAR(true, bitmap_findContinousAt(&b, STATE_2, 3, 7));
    bitmap_setContinous(&b, 7, 3, STATE_4);

    EQ_SCALAR(false, bitmap_findContinousAt(&b, STATE_2, 3, 7));
    EQ_SCALAR(false, bitmap_findContinousAt(&b, STATE_2, 1, 8));

    U8 final[] = {0xFF,0xFF,0xFF};
    EQ_MEM(bitmap, final, sizeof(bitmap));

    END();
}

/**************************************************************************************************
 * One bitmap memory used by two Bitmap objects
**************************************************************************************************/
TEST(bitmap, bitmap_splited_mustpass)
{
    Bitmap low =  {&bitmap[0], 1 /*byte*/, 2, isValid};
    Bitmap high = {&bitmap[1], 2 /*byte*/, 2, isValid};

    EQ_SCALAR(true, bitmap_setContinous(&low, 0, 3, STATE_2));
    EQ_SCALAR(true, bitmap_setContinous(&high, 0, 5, STATE_2));

    U8 final[] = {0xD5,0x55,0xFD};
    EQ_MEM(bitmap, final, sizeof(bitmap));

    // Being 1 byte, low has a capacity of 4 states (0 to 3).
    EQ_SCALAR(4, BITMAP_CAPACITY(&low));
    panic_invoked = false;
    bitmap_setContinous(&low, 4, 1, STATE_1);
    EQ_SCALAR(panic_invoked, true);

    // Being 2 byte, high has a capacity of 8 states (0 to 7).
    EQ_SCALAR(8, BITMAP_CAPACITY(&high));
    panic_invoked = false;
    bitmap_setContinous(&high, 8, 1, STATE_1);
    EQ_SCALAR(panic_invoked, true);

    END();
}

void reset() {
    memset(bitmap, 0xFF, ARRAY_LENGTH(bitmap));
    panic_invoked = false;
}

int main() {
    bitmap_bitsPerState_invalid_mustfail();
    bitmap_null_mustfail();
    zero_length_mustfail();
    invalid_state_mustfail();
    set_large_index_mustfail();
    set_not_allowed_mustfail();
    set_allow_null_mustfail();
    set_valid_states_mustpass();
    set_all_mustpass();
    set_laststate_mustpass();
    set_multiple_mustpass();
    findContinous_mustpass();
    findContinousAt_mustpass();
    get_mustpass();
    bitmap_splited_mustpass();
}
