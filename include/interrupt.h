/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - Cross Platform Kernel - Interrupt and exception handlers
* --------------------------------------------------------------------------------------------------
*/

#ifndef INTERRUPT_H
#define INTERRUPT_H

#if defined(__i386__) || (defined(UNITTEST) && ARCH == x86)
    #include <x86/interrupt.h>
#endif

#endif // INTERRUPT_H
