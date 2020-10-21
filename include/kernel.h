
#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <ints.h>

#ifdef __linux__
//#error "You are targetting Linux. Which is wrong!"
#endif 

#ifndef __i386__
#error "You are not targetting i386."
#endif

#ifdef __i386__
    #include <x86/kernel.h>
#endif

#endif
