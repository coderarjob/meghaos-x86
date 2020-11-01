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

#define VGA_COLUMNS 80
#define VGA_ROWS    25
#define VGA_MAX_INDEX (VGA_COLUMNS * VGA_ROWS -1)

#define CRTC_BLINK_ENABLED  (1<<0)
#define CRTC_IOAS_SET       (1<<1)

#define crt_addr_port  (crtc_flags & CRTC_IOAS_SET)?0x3D4:0x3B4
#define crt_data_port  (crtc_flags & CRTC_IOAS_SET)?0x3D5:0x3B5
#define crt_read(i,v)  do{outb(crt_addr_port,i);inb(crt_data_port,v);}while(0)

static u8 row, column,
          crtc_flags, text_attr;

static u16 *vgab = (u16 *)0xB8000;

static void update_cursor();

/* Display ioctrl */
void kdisp_ioctl(u8 request, ...)
{
    va_list l;
    va_start(l, request);
    u8 *p;
    u8 v;

    switch(request){
        case DISP_SETATTR:
            text_attr = va_arg(l,int);
            if ((crtc_flags & CRTC_BLINK_ENABLED) == 0) 
                text_attr &= 0x7F;
            break;
        case DISP_GETATTR:
            p = va_arg(l,u8 *);
            *p = text_attr;
            break;
        case DISP_SETCOORDS:
            v = va_arg(l,int);       // ROW
            row = v;

            v = va_arg(l,int);       // COLUMN
            column = v;

            update_cursor();
            break;
        case DISP_GETCOORDS:
            p = va_arg(l,u8 *);       // ROW
            *p = row;

            p = va_arg(l,u8 *);       // COLUMN
            *p = column;
            break;
    };
}

/* Finds the where the next character should go and places the cursor there.*/
void kdisp_init()
{
    // ------------------------------------------------------------
    // Scan the vga buffer to find where the next character should go.
    u16 s,          // Current Scan index.
        h;          // Index where the last non space character.

    for (s = 0, h = 0; s <= VGA_MAX_INDEX; s++)
        if (*(u8 *)(vgab+s) != ' ') h = s;

    // If the whole screen is full, we scroll up one line.
    if (++h > VGA_MAX_INDEX)
        kdisp_scrolldown();

    // ------------------------------------------------------------
    // Set the row and column and defaults.
    row = h / VGA_COLUMNS;
    column = h % VGA_COLUMNS;
    update_cursor();

    text_attr = LIGHT_GRAY;                 // LIGHT_GRAY on BLACK

    // ------------------------------------------------------------
    u8 v;

    // 1. Check if IOAS bit is set by reading the External CRTC Register
    // Determines the CRT Address regiser and CRT Registers ports.
    inb(0x3CC,v);
    if (v & 1) crtc_flags |= CRTC_IOAS_SET;

    // ------------------------------------------------------------
    // 3. Check if blink is enabled.
    crt_read(0x10,v);
    if (v >> 3 & 1) crtc_flags |= CRTC_BLINK_ENABLED;
}

void kdisp_scrolldown()
{
    //Copy 2nd line to the 1st line and so on until 24th line
    //TODO: Implement using memcpy

    u16 *p = vgab,                         // Word copied to
        *n = &vgab[VGA_COLUMNS],           // Word copied from
        *e = &vgab[VGA_ROWS*VGA_COLUMNS];  // End byte location.

    for (; n < e; n++, p++)
        *p = *n; 

    for (; p < n ; p++)
        *p = (text_attr<<8|' ');

    // Move cursor one row up. Keeps the column same.
    row -=1;
    column = 0;
    update_cursor();
}

static void update_cursor()
{
    // Set the cursor location.
    u16 index = row * VGA_COLUMNS + column;

    outb(0x3D4, 0xf);
    outb(0x3D5, index & 0xFF);

    outb(0x3D4, 0xe);
    outb(0x3D5, index>>8); 
}

/* Prints an ASCII character on the VGA text mode frame buffer
 * and increments the pointer to it. */
void kdisp_putc(char c)
{
    u16 index;
    switch (c){
        case '\r':
            column = 0;
            break;
        case '\n':
            row++;
            break;
        default:
            index = row * VGA_COLUMNS + column;
            vgab[index] = (text_attr << 8 | c);
            column++;
            break;
    }

    if (column >= VGA_COLUMNS) {
        row++;
        column = 0;
    }

    if (row >= VGA_ROWS)
        kdisp_scrolldown();

    update_cursor();
}
