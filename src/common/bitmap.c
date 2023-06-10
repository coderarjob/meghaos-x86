/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - Common - Bitmap
*
* Bitmap implementation for use in the OS.
*
* NOTE: Being a common component, the methods must detect error conditions and report to the
* caller? Or should panic! or at a later stage stop the process?
*
* Bitmap:
* It stores states, the interpretation of which is let to the user of bitmap. Each state can be
* represented by more than one bit. In the example below, each state is 2 bits.
*
*    7    6    5    4    3    2    1    0
*  +----+----+----+----+----+----+----+----+
*  | State 3 | State 2 | State 1 | State 0 |
*  +----+----+----+----+----+----+----+----+
* --------------------------------------------------------------------------------------------------
*/

#include <utils.h>
#include <types.h>
#include <kerror.h>
#include <common/bitmap.h>

/***************************************************************************************************
 * Searches bitmap for continous `len` number of continous blocks with `state` state starting from
 * index 'indexAt' in the bitmap.
 * TODO: User must get lock on the bitmap before calling this function.
 *
 * @Input b          Bitmap
 * @Input state      Searches for a continous section with this state.
 * @Input len        This many consecutive blocks.
 * @return           On succes returns KERNEL_EXIT_SUCCESS, else KERNEL_EXIT_FAILURE.
 **************************************************************************************************/
INT bitmap_findContinousAt (Bitmap *b, BitmapState state, UINT len, UINT indexAt)
{
    k_assert(state < power_of_two(b->bitsPerState), "Invalid  state");

    UINT i = 0;
    for (; i < len && bitmap_get(b, i + indexAt) == state; i++)
        k_assert(i < BITMAP_ITEM_COUNT(b), "Index out of bounds.");

    // If the inner loop itterated `len` number of types, then we have
    // found out contious block.
    return (i == len ) ? KERNEL_EXIT_SUCCESS : KERNEL_EXIT_FAILURE;
}

/***************************************************************************************************
 * Searches bitmap for continous `len` number of continous blocks with `state` state.
 * TODO: User must get lock on the bitmap before calling this function.
 *
 * @Input b          Bitmap
 * @Input state      Searches for a continous section with this state.
 * @Input len        This many consecutive blocks.
 * @return           On succes returns the index of the first item in the block, otherwise returns
 *                   KERNEL_EXIT_FAILURE.
 **************************************************************************************************/
INT bitmap_findContinous (Bitmap *b, BitmapState state, UINT len)
{
    k_assert(state < power_of_two(b->bitsPerState), "Invalid  state");
    UINT count = BITMAP_ITEM_COUNT(b);

    for (UINT index = 0; index < count; index++)
        if (bitmap_findContinousAt(b, state, len, index) != KERNEL_EXIT_FAILURE)
            return (INT)index;

    // No contigous block found
    return KERNEL_EXIT_FAILURE;
}

/***************************************************************************************************
 * Gets the bit at index
 *
 * @Input b          Bitmap
 * @Input index      Index starts from 0. What each bit represents is upto the user.
 * @return           On succes returns bitmap item at the index, otherwise returns KERNEL_EXIT_FAILURE.
 **************************************************************************************************/
INT bitmap_get (Bitmap *b, UINT index)
{
    /*if (index == BITMAP_ITEM_COUNT(b))
        RETURN_ERROR(BITMAP_ERR_OUTSIDE_RANGE, false);*/

    UINT byteIndex = index / BITMAP_STATES_PER_BYTE(b);
    UINT bitIndex = (index % BITMAP_STATES_PER_BYTE(b)) * bitmap_bitsPerState(b);

    return ((b->bitmap[byteIndex] >> bitIndex) & (INT)BITMAP_STATE_MASK(b));
}

/***************************************************************************************************
 * Sets 'len' number of items starting at 'index' to 'state'.
 * TODO: User must get lock on the bitmap before calling this function.
 *
 * @Input b             Bitmap
 * @Input index         Index starts from 0. What each bit represents is upto the user.
 * @Input len           Number of items to change.
 * @Input state         State to set to. Validity is checked before making a change.
 * @return              Returns KERNEL_EXIT_SUCCESS on success, else KERNEL_EXIT_FAILURE.
 **************************************************************************************************/
INT bitmap_setContinous (Bitmap *b, UINT index, UINT len, BitmapState state)
{
    k_assert(state < power_of_two(b->bitsPerState), "Invalid  state");

    for (; len > 0 && b->allow(index, (BitmapState)bitmap_get(b, index), state); len--, index++)
    {
        k_assert(index < BITMAP_ITEM_COUNT(b), "Index out of bounds.");

        UINT byteIndex = index / BITMAP_STATES_PER_BYTE(b);
        UINT bitIndex = (index % BITMAP_STATES_PER_BYTE(b)) * bitmap_bitsPerState(b);
        b->bitmap[byteIndex] &= (BitmapState)~(BITMAP_STATE_MASK(b) << bitIndex);
        b->bitmap[byteIndex] |= (BitmapState)(state << bitIndex);
    }

    return (len == 0) ? KERNEL_EXIT_SUCCESS : KERNEL_EXIT_FAILURE;
}
