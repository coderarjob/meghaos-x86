## Megha Operating System V2 - x86
## Naming Conventions of Variables and Functions
------------------------------------------------------------------------------
_2nd November 2020_

### Header files hierarchy

The idea is that the Kernel and other source files should include header files
that are build in a 'platform independent' way. This independence is due to the
header files themselves is responsible to include the necessary 'Platform
dependent' header files. 

**FILE: io.h**
```
.
.
#ifdef __i386__
    #include<x86/io.h>
#endif
.
.
```

This technique works due only when there is a parent header file, that can 
include the platform dependent headers -  `io.h` includes `x86/io.h`in
_i386_ systems, `types.h` includes `x86/types.h` in similar way.

What to do when there is no parent header file? In that case

1. We can include it selectively in the most logical place possible. (Next best
   parent header file). For example: `x86/vgatext.h` is included in `disp.h`

2. Can be included in the platform specific `kernel.h` file.
   For example: `x86/vgatext.h` is included in `kernel.h`

So which option to choose? It depends on a little foresight. 

* If **similar files will exist for other architectures, then option 1, is right.**
  For example: It is my foresight that there will be files like `vgatext.h` and
  `memloc.h` in other architectures as will. So I decided to include them in
  the `disp.h` and `mem.h` respectively. In arm for example we will have
  something like this in `mem.h`

**FILE: mem.h**
```
.
.
#ifdef __i386__
    #include<x86/memloc.h>
#endif

#ifdef __arm__
    #include<arm/memloc.h>
#endif
.
.
.
```

That is all. Current hierarchy is follows:

```
    kernel.h
        |
        |___types.h 
        |       |___x86/types.h
        |
        |___disp.h  
        |       |___x86/vgatext.h
        |
        |___mem.h
        |       |___x86/memloc.h
        |
        |___io.h    
        |       |___x86/io.h
        |
        |___x86/kernel.h
```

Keep it simple.

------------------------------------------------------------------------------
_1st November 2020_

I have been following the [https://github.com/guilt/KOS][KOS], for its 
simplicity. For example routine that prints one character directly on the VGA 
buffer is named `putc`. I think this is too simple for my purpose, as the 
standard `putc` function puts characters into a file descriptor not to VGA 
buffer. We will see this pattern over and over that implementation of kernel 
function and userland functions are different. 

### Functions

* Functions that are meant to be used only in the kernel will have `k` either 
  in prefix or suffix. 
* The function names generally should not tie with implementation.

  1. `kvga_putc()` is bad.
     `kdisp_putc()` is better. As it does not tie the name with VGA. What is CGA
     is used?
  2. `kmalloc()` instead of `malloc()` in kernel implementation.

* Static functions are also named normally with no indication of scope.
* Variables and function with `__` in front are for reserved for special
  purposes.
* Global helper functions (functions that exist to be called from another
  macro) should have `__` in front. For example: `__kpanic()` function is the
  helper function, being called from the `kpanic()` macro.

### Variables

* Global extern variables are named with a prefix `g_`. All external variables
  must be explicitly declared in the file that they are being used, with the
  `extern` keyword.
* Global static variables are named normally.

### Header files

* We do not want the kernel code to include too many header files all the time.
  Common header files should be included into the `kernel.h`. 
* Platform specific header files should exist inside a folder properly named. 
  **Example:** x86 `kernel.h` should be placed inside `include/x86/kernel.h`.

