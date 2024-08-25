# Megha Operating System V2 - x86
------------------------------------------------------------------------------

## VESA VBE Graphics mode
categories: feature, x86

For builds with graphics enabled, bootloader `boot1` first checks for availability of VBE functions
and then selects the first graphics mode which matches every search criteria.

| Criteria                  | Value                                           |
|---------------------------|-------------------------------------------------|
| X Resolution              | 800                                             |
| Y Resolution              | 600                                             |
| Bits/pixel                | 32                                              |
| Mode attributes           | Graphics mode and Linear frame buffer           |
| Memory model              | Packed pixel or DirectColor                     |
| RGBPosition (DirectColor) | Position R,G,B fields positions to be 16,8 & 0. |

32 color is selected just because its supported more or less by every emulator and modern PCs. Also
it is easier to adopt drawing functions for 24 and 8 bit color if they were first implemented for 32
bit one.

24 bit color and 8 bit colors are not always supported either by the actual hardware or by
emulators. VESA modes which used to be 24 bits now are 32 bits in modern systems For example, mode
0x115 used to be 800 x 600, 24bpp, but on my laptop the same mode is 800 x 600, 32bpp.

800 x 600, 8bpp is also not a standard SVGA mode and thus support is not always present. Another
problem with 8 bit color is due to it being a indexed color, the color palette may not be all same
in every system. For example VirtualBox have a different palette all together and colors all look
wrong..

Note that resolution is less of a problem, we can change that easily if the support of color depth
is present. So if we want to run MeghaOS in 1024 x 768 resolution, we just have to mention the
resolution in 'mos.inc' and 'x86/config.h' files.

If you want to try 8 bit or 24 bit color, change corresponding settings in the above mentioned two
files.
