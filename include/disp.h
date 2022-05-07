/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Printing on Screen.
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/
#ifndef __DISPTEXT_H__
#define __DISPTEXT_H__

#include <types.h>

#if defined(__i386__) || (defined(UNITTEST) && ARCH == x86)
    #include <x86/vgatext.h>
#endif

typedef enum DisplayControls 
{
    DISP_GETCOORDS, 
    DISP_SETCOORDS, 
    DISP_COORDS_MAX,
    DISP_SETATTR, 
    DISP_GETATTR
} DisplayControls;

/* Display ioctrl
 * Returns:
 * On Success, ERR_NONE (0) is returned.
 * On Failure, k_errorNumber is set and -1 is returned.
 */
INT kdisp_ioctl (U8 request, ...);

/* Scrolls down one line. */
void kdisp_scrollDown ();

/* Finds the where the next character should go and places the cursor there.*/
void kdisp_init ();

/* Prints an ASCII character on the VGA text mode frame buffer
 * and increments the pointer to it. */
void kdisp_putc (CHAR c);

#endif // __DISPTEXT_H__
