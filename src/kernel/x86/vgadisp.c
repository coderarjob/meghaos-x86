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

#define TABSTOP             4U
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

static UINT row, column, crtc_flags, text_attr;
static U16 *vgab;
static bool is_initialized;

static void s_updateCursor ();

/***************************************************************************************************
 * Sets or gets display attributes.
 *
 * @param request   Determines what operation or whether to get/set attributes. One of the member in
 *                  DisplayControls enum.
 * @param ...       Arguments for the specified operation. Provides input (is required)/output for
 *                  the specified operation.
 * @returns         On success, EXIT_SUCCESS is returned, otherwise EXIT_FAILURE.
 **************************************************************************************************/
INT kdisp_ioctl (INT request, ...)
{
    va_list l;
    va_start (l, request);
    UINT *p;

    switch (request){
        case DISP_SETATTR:
            text_attr = va_arg (l,UINT);
            if (text_attr > 255)
                goto error_invalid_range;

            if ((crtc_flags & CRTC_BLINK_ENABLED) == 0) text_attr &= 0x7F;
            break;
        case DISP_GETATTR:
            p = va_arg (l,UINT *);
            *p = text_attr;
            break;
        case DISP_SETCOORDS:
            row = va_arg (l,UINT);       // ROW
            column = va_arg (l,UINT);    // COLUMN

            if (row > MAX_VGA_ROWS || column > MAX_VGA_COLUMNS)
                goto error_invalid_range;

            s_updateCursor ();
            break;
        case DISP_GETCOORDS:
            p = va_arg (l,UINT *);       // ROW
            *p = row;

            p = va_arg (l,UINT *);       // COLUMN
            *p = column;
            break;
    };
    va_end (l);
    return EXIT_SUCCESS;

error_invalid_range:
    va_end(l);
    RETURN_ERROR (ERR_INVALID_RANGE, EXIT_FAILURE);

}

/***************************************************************************************************
 * Finds the where the next character should go and places the cursor there.
 *
 * @returns     Nothing
 **************************************************************************************************/
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
    s_updateCursor ();

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

    // ------------------------------------------------------------
    // Initialization is done.
    is_initialized = true;
}

/***************************************************************************************************
 * Scrolls down the contents on the display by one line.
 *
 * Performs a software scrolling, where a new empty line is created at the very last row of the
 * display. Does not make any sense if called when the cursor is not on the last row of the display.
 *
 * @returns     Nothing
 **************************************************************************************************/
void kdisp_scrollDown ()
{
    // Copy 2nd line to the 1st line and so on until the 2nd last line.
    U16 *p = vgab,                           // Word copied to
        *n = &vgab[VGA_COLUMNS],             // Word copied from
        *e = &vgab[VGA_ROWS * VGA_COLUMNS];  // End byte location.

    for (; n < e; n++, p++)
        *p = *n; 

    // Empty the last line.
    for (; p < n ; p++)
        *p = (U16)(text_attr<<8|' ');

    // Move cursor one row up. Keeps the column same.
    // TODO: Can we not set the row to the last row??
    if (row > 0)
        row = (U8)(row - 1);
    column = 0;
    s_updateCursor ();
}

/***************************************************************************************************
 * Updates VGA text mode cursor depending on the global row and column variables.
 * @returns     Nothing
 **************************************************************************************************/
static void s_updateCursor ()
{
    // Set the cursor location.
    UINT index = row * VGA_COLUMNS + column;

    crt_write (0xf, index & 0xFF);
    crt_write (0xe, index>>8);
}

/***************************************************************************************************
 * Puts an ASCII character on the VGA text mode frame buffer and increments the pointer to it.
 *
 * @param c     Character to put. Escape characters like \n, \r and \t are handled here.
 * @returns     Nothing
 **************************************************************************************************/
void kdisp_putc (CHAR c)
{
    UINT index;
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
            vgab[index] = (U16)(text_attr << 8 | (UINT)c);
            column++;
            break;
    }

    if (column >= VGA_COLUMNS) {
        row++;
        column = 0;
    }

    if (row >= VGA_ROWS)
        kdisp_scrollDown ();

    s_updateCursor ();
}

/***************************************************************************************************
 * Returns status of text display initialization.
 *
 * @return true if kdisp_init has been called.
 **************************************************************************************************/
bool kdisp_isInitialized ()
{
    return is_initialized;
}
