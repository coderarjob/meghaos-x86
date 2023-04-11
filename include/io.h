/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - Cross platform Kernel - Basic I/O routines.
*
* This is not and never going to be a full featured VGA text mode driver.
* Functions here will be limited to prints or printf like functions.
* NOTE: Actual VGA driver will be a server in User Mode.
* ---------------------------------------------------------------------------
*
* Dated: 24th October 2020
*/

#ifndef IO_H
#define IO_H

#include <buildcheck.h>

#if defined(__i386__) || (defined(UNITTEST) && ARCH == x86)
    #include <x86/io.h>
#endif

#endif // IO_H
