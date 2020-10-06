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
/*
 * Displays a 16 bit number in Hexadecimal representation on the screen.
 * */
void kprinthex(u16 num)
{
    char *hexchars = "0123456789ABCDEF";
    char output[5] = {0};

    for(int i = 3; i >= 0; i--, num >>= 4)
        output[i] = hexchars[num & 0xF];

    prints("0x");
    prints(output);
}

/*
 * Prints a ASCIZ string on the VGA text mode frame buffer.
 * */
void prints(char *s)
{
    static u8 *vgabuff = (u8 *)0xb8000;

    for(; *s; s++, vgabuff+=2) {
        *vgabuff = *s;
        *(vgabuff+1) = 0xF;
    }
}

