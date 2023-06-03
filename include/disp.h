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
#ifndef DISPTEXT_H
#define DISPTEXT_H

#include <stddef.h>
#include <types.h>
#include <stdarg.h>
#include <buildcheck.h>

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

/***************************************************************************************************
 * Sets or gets display attributes.
 *
 * @param request   Determines what operation or whether to get/set attributes. One of the member in
 *                  DisplayControls enum.
 * @param ...       Arguments for the specified operation. Provides input (is required)/output for
 *                  the specified operation.
 * @returns         On success, EXIT_SUCCESS is returned, otherwise EXIT_FAILURE.
 **************************************************************************************************/
INT kdisp_ioctl (INT request, ...);

/***************************************************************************************************
 * Scrolls down the contents on the display by one line.
 *
 * Performs a software scrolling, where a new empty line is created at the very last row of the
 * display. Does not make any sense if called when the cursor is not on the last row of the display.
 *
 * @returns     Nothing
 **************************************************************************************************/
void kdisp_scrollDown ();

/***************************************************************************************************
 * Finds the where the next character should go and places the cursor there.
 *
 * @returns     Nothing
 **************************************************************************************************/
void kdisp_init ();

/***************************************************************************************************
 * Puts an ASCII character on the display text mode frame buffer and increments the pointer to it.
 *
 * @param c     Character to put. Escape characters like \n, \r and \t are handled here.
 * @returns     Nothing
 **************************************************************************************************/
void kdisp_putc (CHAR c);

/***************************************************************************************************
 * Returns status of text display initialization.
 *
 * @return true if kdisp_init has been called.
 **************************************************************************************************/
bool kdisp_isInitialized ();

INT kearly_printf (const CHAR *fmt, ...);
INT kearly_snprintf (CHAR *dest, size_t size, const CHAR *fmt, ...);
INT kearly_vsnprintf (CHAR *dest, size_t size, const CHAR *fmt, va_list l);

#endif // DISPTEXT_H
