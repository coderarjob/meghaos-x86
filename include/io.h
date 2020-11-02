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

#ifndef __IO_H__
#define __IO_H__

#ifdef __i386__
    #include <x86/io.h>
#endif

#endif // __IO_H__
