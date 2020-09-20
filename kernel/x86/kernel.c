#include <ints.h>

void prints(char *s);
void printhex(u16 num);

void __main()
{
    prints("Kernel ready..");
    while(1);
}

void prints(char *s)
{
    u8 *vgabuff = (u8 *)0xb8000;

    for(; *s; s++, vgabuff+=2) {
        *vgabuff = *s;
        *(vgabuff+1) = 0xF;
    }
}

