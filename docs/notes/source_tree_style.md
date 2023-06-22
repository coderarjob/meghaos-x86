## Megha Operating System V2
------------------------------------------------------------------------------

## Source tree conventions
categories: guideline, independent, obsolete
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
