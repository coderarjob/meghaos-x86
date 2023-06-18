/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - Common - Bitmap
*
* Bitmap implementation for use in the OS.
*
* Bitmap:
* It stores states, the interpretation of which is let to the user of bitmap. Each state can be
* represented by more than one bit. In the example below, each state is 2 bits.
* These states are addresses with their index in the bitmap. Indices start from 0.
*
*    7    6    5    4    3    2    1    0
*  +----+----+----+----+----+----+----+----+
*  | State 3 | State 2 | State 1 | State 0 |
*  +----+----+----+----+----+----+----+----+
*
* NOTE: Being a common component, the methods must detect error conditions and report to the
* caller? If any function gets wrong input, this is a symptom of some wrong calculation elsewhere.
* The only safe thing to do is to halt. Just reporting in the hope that the caller will deal with it
* may not be a good idea, we should fail fast at the first sign of problem.
*
* NOTE: If a function reports a boolean, then it should report false on failure. The above
* statement only deals with invalid/wrong/error conditions.
* --------------------------------------------------------------------------------------------------
*/

#include <utils.h>
#include <types.h>
#include <kerror.h>
#include <common/bitmap.h>

/***************************************************************************************************
 * Checks if a section in the bitmap exists starting at index `indexAt` having at least `len`
 * number of states with value of `state`.
 * TODO: User must get lock on the bitmap before calling this function.
 *
 * @Input b          Bitmap
 * @Input state      Searches for this state in the bitmap.
 * @Input len        This many consecutive states. Must be > 0.
 * @Input indexAt    Start index in the bitmap. First index is 0.
 * @return           On success returns true, false otherwise.
 **************************************************************************************************/
bool bitmap_findContinousAt (Bitmap *b, BitmapState state, UINT len, UINT indexAt)
{
    k_assert(b != NULL, "Cannot be null");
    k_assert(state < BITMAP_MAX_STATE(b), "Invalid state");
    k_assert(len > 0, "Must be > zero");
    k_assert ((indexAt + len - 1) < BITMAP_CAPACITY(b), "Index out of bounds.");

#ifdef UNITTEST
    // Do not proceed if one the asserts has failed.
    if (panic_invoked) return false;
#endif

    UINT i = 0;
    for (; i < len && bitmap_get(b, i + indexAt) == state; i++);

    // If the inner loop iterated `len` number of types, then we have
    // found our continuous block.
    return (i == len) ? true : false;
}

/***************************************************************************************************
 * Searches bitmap for a section with at least `len` number of continuous states with `state` value.
 * TODO: User must get lock on the bitmap before calling this function.
 *
 * @Input b          Bitmap
 * @Input state      Searches for this state in the bitmap.
 * @Input len        This many consecutive states. Must be > 0.
 * @return           On success returns the first index in the section, otherwise returns
 *                   KERNEL_EXIT_FAILURE.
 **************************************************************************************************/
INT bitmap_findContinous (Bitmap *b, BitmapState state, UINT len)
{
    k_assert(b != NULL, "Cannot be null");
    k_assert(state < BITMAP_MAX_STATE(b), "Invalid state");
    k_assert(len > 0, "Must be > zero");

#ifdef UNITTEST
    // Do not proceed if one the asserts has failed.
    if (panic_invoked) return KERNEL_EXIT_FAILURE;
#endif

    UINT count = BITMAP_CAPACITY(b);

    for (UINT index = 0; index < count; index++)
        if (bitmap_findContinousAt(b, state, len, index))
            return (INT)index;

    // No contiguous block found
    return KERNEL_EXIT_FAILURE;
}

/***************************************************************************************************
 * Gets the bit at index
 *
 * @Input b          Bitmap
 * @Input index      Index into the bitmap. First index is 0.
 * @return           On success returns bitmap item at the index, otherwise returns
 *                   KERNEL_EXIT_FAILURE.
 **************************************************************************************************/
BitmapState bitmap_get (Bitmap *b, UINT index)
{
    k_assert(b != NULL, "Cannot be null");
    k_assert (index < BITMAP_CAPACITY(b), "Index out of bounds.");

#ifdef UNITTEST
    // Do not proceed if one the asserts has failed.
    if (panic_invoked) return (BitmapState)KERNEL_EXIT_FAILURE;
#endif

    UINT byteIndex = index / BITMAP_STATES_PER_BYTE(b);
    UINT bitIndex = (index % BITMAP_STATES_PER_BYTE(b)) * bitmap_bitsPerState(b);

    return (BitmapState)((b->bitmap[byteIndex] >> bitIndex) & (INT)BITMAP_STATE_MASK(b));
}

/***************************************************************************************************
 * Sets 'len' number of items starting at 'index' to 'state'.
 * TODO: User must get lock on the bitmap before calling this function.
 *
 * @Input b             Bitmap
 * @Input index         Index into the bitmap. First index is 0.
 * @Input len           Number of consecutive states in the bitmap to set. Must be > 0.
 * @Input state         State to set to.
 * @return              Returns true on success, else false.
 **************************************************************************************************/
bool bitmap_setContinous (Bitmap *b, UINT index, UINT len, BitmapState state)
{
    k_assert(b != NULL, "Cannot be null");
    k_assert(b->allow != NULL, "Cannot be null");
    k_assert(state < BITMAP_MAX_STATE(b), "Invalid state");
    k_assert(len > 0, "Must be > zero");
    k_assert((index + len - 1) < BITMAP_CAPACITY(b), "Index out of bounds.");

#ifdef UNITTEST
    // Do not proceed if one the asserts has failed.
    if (panic_invoked) return false;
#endif

    for (; len > 0 && b->allow(index, (BitmapState)bitmap_get(b, index), state); len--, index++)
    {
        UINT byteIndex = index / BITMAP_STATES_PER_BYTE(b);
        UINT bitIndex = (index % BITMAP_STATES_PER_BYTE(b)) * bitmap_bitsPerState(b);
        b->bitmap[byteIndex] &= (BitmapState)~(BITMAP_STATE_MASK(b) << bitIndex);
        b->bitmap[byteIndex] |= (BitmapState)(state << bitIndex);
    }

    return (len == 0) ? true : false;
}
