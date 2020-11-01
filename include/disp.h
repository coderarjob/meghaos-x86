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


enum disp_ioctl {DISP_GETCOORDS, 
                 DISP_SETCOORDS, 
                 DISP_COORDS_MAX,
                 DISP_SETATTR, 
                 DISP_GETATTR};

/* Display ioctrl */
void kdisp_ioctl(u8 request, ...);

/* Scrolls down one line. */
void kdisp_scrolldown();

/* Finds the where the next character should go and places the cursor there.*/
void kdisp_init();

/* Prints an ASCII character on the VGA text mode frame buffer
 * and increments the pointer to it. */
void kdisp_putc(char c);

#endif // __DISPTEXT_H__
