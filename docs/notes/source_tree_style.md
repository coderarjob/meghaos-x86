## Megha Operating System V2
------------------------------------------------------------------------------

## Unittests file system hierarchy in MeghaOS
categories: guideline, independent
_05 May 2022_

The mock source, header and compiled object files are related one to one to the original C file.
Because we will have multiple files with the same name, (this is because, source files with same 
name will/may exist for other architectures) it is important to preserve and clone the hierarchy 
of the original C file for the other mentioned relatives.

Here is an example.

```
Original C file       src/kernel/x86/vgadisp.c
Mock C file           src/mock/kernel/x86/vgadisp.c
Mock H file           include/mock/kernel/x86/vgadisp.h
Compiled object file  build/obj/mock/kernel/x86/vgadisp.o [*]
```

Similarly, the File Under Test (FUT) and compiled object file, needs to be under the same directory
hierarchy for the same reason.

Note that the same source file (which in the context of Unittests, we are calling FUT) is 
compiled twice, one for the OS by the cross compiler, and another to be part of the unittest, 
compiled by the host compiler.

Here we are talking about the former.

```
FUT C file          src/kernel/printk.c
FUT C object file   build/obj/unittests/kernel/printk.o [*]
                    build/obj/fut/kernel/printk.o       (this may be a better location)
```

The unittest C files are treated similarly. Most of these tests are architecture independent, 
so most will not be inside a 'ARCH' folder. However if there is such a need to place the source 
inside a folder (architecture dependent test, or sometime else) then hierarchy must be maintained 
for the object and executable file (for the same reason mentioned above)

```
Unittest C file         src/unittests/printk_test.c
Object file             build/obj/unittests/printk_test.o
Host executable file    build/bin/unittests/printk_test
```

As MeghaOS may extend to other cpu architectures, it is important to think about the location and
responsibilities of the build scripts.

Separating them for each architecture, will help maintain them better. A single build script for 
every supported architecture will be a nightmare to maintain. Without separation, there will 
be many reason to change that single file, making the system fragile.

So this is essential.

```
build.sh
    src/unittests/build.sh
        src/mock/build.sh
             src/mock/x86/build.sh
        src/unittests/build_fut.sh
             src/unittests/x86/build_fut.sh
        src/unittests/x86/build.sh
```

Current implementation have build scripts at below locations.

```
build.sh
    src/unittests/build.sh (Currently this builds, FUT files, unittest and mock C files)
```
PS:
[*] Not done, this is currently not hierarchical.

------------------------------------------------------------------------------

## Build scripts in MeghaOS
categories: guideline, independent
_06 May 2022_

All the build scripts are stored in separate tree. This makess less clutter in the src folder,
which are only for source files anyways. Currently with just one 'build.sh' script in each of the
subfolders in src, is not that bad, but if the number of scripts somehow grows, it will definately
clutter.

```
    build.sh                -> Starts the build process, by calling the common/build.sh.
    build/scripts
        build.sh            -> Architecture independent build of the os source files.
        build-fut.sh
        build-mock.sh
        build-unittests.sh
    build/scripts/common
        env-vars.sh
        functions.sh
    build/scripts/x86
        kernel.ld
        env-vars.sh
        functions.sh
        build-fut.sh
        build-mock.sh
        build-unittests.sh
        build.sh            -> Architecture sepecific build. In the example for the x86 arch.

```

The output from the build scripts are stored in another separate tree. This means at the start of
the build, 'build/x86' can be safely deleted and all the files and folders will be created by the
scripts.

```
    build/x86/
        diskimage
        list
        obj
            fut
            mock
            unittests
```

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
