/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Basic VGA Text driver.
*
* This is not and never going to be a full featured VGA text mode driver.
* Functions here will be limited to prints or printf like functions.
* NOTE: Actual VGA driver will be a server in User Mode.
* ---------------------------------------------------------------------------
*
* Dated: 6th October 2020
*/

#include <kernel.h>
#include <stdarg.h>

#define TABSTOP             4
#define VGA_COLUMNS         MAX_VGA_COLUMNS
#define VGA_ROWS            MAX_VGA_ROWS
#define VGA_MAX_INDEX       (VGA_COLUMNS * VGA_ROWS -1)

#define CRTC_BLINK_ENABLED  (1<<0)
#define CRTC_IOAS_SET       (1<<1)

#define crt_addr_port  (crtc_flags & CRTC_IOAS_SET) ? 0x3D4 : 0x3B4
#define crt_data_port  (crtc_flags & CRTC_IOAS_SET) ? 0x3D5 : 0x3B5

#define crt_read(i,v)  do{                              \
                            outb (crt_addr_port,i);     \
                            inb (crt_data_port,v);      \
                       }while (0)

#define crt_write(i,v) do{                              \
                            outb (crt_addr_port,i);     \
                            outb (crt_data_port,v);     \
                       }while (0)

static U8 row, column,
          crtc_flags, 
          text_attr;

static U16 *vgab;

static void update_cursor ();

/* Display ioctrl
 * Returns:
 * On Success, ERR_NONE (0) is returned.
 * On Failure, k_errorNumber is set and -1 is returned.
 */
INT kdisp_ioctl (U8 request, ...)
{
    va_list l;
    va_start (l, request);
    INT *p;
    INT v;

    switch (request){
        case DISP_SETATTR:
            v = va_arg (l,INT);
            if (v < 0 || v > 255)
            {
                va_end(l);
                RETURN_ERROR (ERR_INVALID_RANGE, -1);
            }

            text_attr = (U8)v;
            if ((crtc_flags & CRTC_BLINK_ENABLED) == 0) text_attr &= 0x7F;
            break;
        case DISP_GETATTR:
            p = va_arg (l,INT *);
            *p = (INT)text_attr;
            break;
        case DISP_SETCOORDS:
            v = va_arg (l,INT);       // ROW
            if (v < 0 || v > MAX_VGA_ROWS)
            {
                RETURN_ERROR (ERR_INVALID_RANGE, -1);
            }
            row = (U8)v;

            v = va_arg (l,INT);       // COLUMN
            if (v < 0 || v > MAX_VGA_COLUMNS)
            {
                RETURN_ERROR (ERR_INVALID_RANGE, -1);
            }
            column = (U8)v;

            update_cursor ();
            break;
        case DISP_GETCOORDS:
            p = va_arg (l,INT *);       // ROW
            *p = (INT)row;

            p = va_arg (l,INT *);       // COLUMN
            *p = (INT)column;
            break;
    };
    va_end (l);
    return ERR_NONE;
}

/* Finds the where the next character should go and places the cursor there.*/
void kdisp_init ()
{ 
    vgab = (U16 *)VIDEO_RAM_LOCATION;
    // ------------------------------------------------------------
    // Scan the vga buffer to find where the next character should go.
    U16 s,          // Current Scan index.
        h;          // Index where the last non space character.

    for (s = 0, h = 0; s <= VGA_MAX_INDEX; s++)
        if (*(U8 *)(vgab + s) != ' ') h = s;

    // If the whole screen is full, we scroll up one line.
    if (++h > VGA_MAX_INDEX)
        kdisp_scrollDown ();

    // ------------------------------------------------------------
    // Set the row and column and defaults.
    row    = (U8)(h / VGA_COLUMNS);
    column = (U8)(h % VGA_COLUMNS);
    update_cursor ();

    text_attr = LIGHT_GRAY;                 // LIGHT_GRAY on BLACK

    // ------------------------------------------------------------
    U8 v;

    // 1. Check if IOAS bit is set by reading the External CRTC Register
    // Determines the CRT Address regiser and CRT Registers ports.
    inb (0x3CC,v);
    if (v & 1) crtc_flags |= CRTC_IOAS_SET;

    // ------------------------------------------------------------
    // 3. Check if blink is enabled.
    crt_read (0x10,v);
    if (v >> 3 & 1) crtc_flags |= CRTC_BLINK_ENABLED;
}

void kdisp_scrollDown ()
{
    //Copy 2nd line to the 1st line and so on until 24th line

    U16 *p = vgab,                           // Word copied to
        *n = &vgab[VGA_COLUMNS],             // Word copied from
        *e = &vgab[VGA_ROWS * VGA_COLUMNS];  // End byte location.

    for (; n < e; n++, p++)
        *p = *n; 

    for (; p < n ; p++)
        *p = (U16)(text_attr<<8|' ');

    // Move cursor one row up. Keeps the column same.
    // TODO: Can we not set the row to the last row??
    if (row > 0)
        row = (U8)(row - 1);
    column = 0;
    update_cursor ();
}

static void update_cursor ()
{
    // Set the cursor location.
    INT index = row * VGA_COLUMNS + column;

    crt_write (0xf, index & 0xFF);
    crt_write (0xe, index>>8);
}

/* Prints an ASCII character on the VGA text mode frame buffer
 * and increments the pointer to it. */
void kdisp_putc (CHAR c)
{
    INT index;
    switch (c){
        case '\r':
            column = 0;
            break;
        case '\n':
            row++;
            break;
        case '\t':
            column += (TABSTOP - (column % TABSTOP));
            break;
        default:
            index = row * VGA_COLUMNS + column;
            vgab[index] = (U16)(text_attr << 8 | c);
            column++;
            break;
    }

    if (column >= VGA_COLUMNS) {
        row++;
        column = 0;
    }

    if (row >= VGA_ROWS)
        kdisp_scrollDown ();

    update_cursor ();
}
