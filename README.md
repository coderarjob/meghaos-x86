# MEGHA OPERATING SYSTEM V2 - x86

Well let me keep things straigt here. I am moving Megha from being a Real Mode
Operating System to a full 32 bit Protected Mode OS. Why you ask?

The idea first came, because while creating the ABI for Real Mode Megha, the
minimum it seems it can be run on is on a 386 - not a 8086. So if I am
building an OS that uses some features of a 386, why run in Real Mode, when the
processor has so much more to offer.

|            Reason               |  Minimum Architecture    |
|---------------------------------|--------------------------|
|  PUSHA, POPA                    |         80186             |
|  IMUL with immediate value      |         80186             |
|  Use of GS, FS registers by Applications and in Kernel.  |  80386  | 
|  Application can use 32 bit registers. |  80386  |
|  ENTER, LEAVE instructions      |            80186             |

Note: FS, GS and 32 bit registers is not used by the Kernel, but the
applicaiton programs that will be written in C and compiled most likely with
GCC. Thus it is the 386 that is the minimum requirement. I could have used
OpenWatcom compiler and that would have worked, but that may force me to stay
using a compiler that is really not the Standard - it will take me away from
GCC or other modern compilers, when I am also trying to find a job here.

So it is going to be a PROTECTED MODE Operaing System, with parts made in C
rather than with only Assembly. It will also help is producing a protable code
- Will help to port Megha to other Processor Atchitecture much easily, than if
  the whole OS is written is Assembly.

## Advantages:
    1. To be able to stay with the C language, now that I am trying to find a
       Job in that language. I tend to forget stuff very quickly, once out of
       sight.
    2. Protable Code. Megha could be transfered to ARM with much less
       challenge.
    3. Being in Protected Mode, C compilers will behave less strange, and GCC,
       GDB can be used.

## Design of the Megha will still have the same Goals 
    - To be Modular and easy to replace parts of it. 
    - Kernel as small as possible with the basics.
    - Drivers etc, will be an extension of the OS, not part of the Kernel
      Maybe loaded by the SHELL.

      SMELLS OF MICRO KERNEL. BUT CAN WE DO WITHOUT SIGNALS?

The end product will be ready for a programmer but not for general use.

## Features:
1. Small modular kernel
2. Modules will be loaded from a text file by the Shell
3. FAT12 Filesystem
4. Kernel will have API for 
   * TSR programs
   * Adding IRQ handler routines.
   * SIGNALS
5. Keyboard, VGA drivers will be loaded by the SHELL and will not be part of
   the kernel. As and when needed, any modules can be replaced or disabled.
6. Will target x86 Protected Mode.
7. VFS, DEVFS and ROM DISKS (Not priority now)

## Kernel design goals
1. Megha is not to use too much x86 specific features, because that will make
   porting difficult. 

## Git Branches
* Master  - Current Release. With Release tags, that mark the different commits.
* Develop - Current Develop branch, where all the feature branches will be
            created and merged into.
* Feature - Lives temporarily and named like `feature-bootloader`.
* Hotfix  - Lives temporarily and named like `hotfix-ls-segfault`.
```
            *HotFix   Master   Develop     *Feature
               |        |           |           |
               |        |           |           |
               |        |<--------->|           |
               |<------>|           |<--------->|
```

## Releases and Versioning:
1. Every commit in the Master branch must have a tag, and this will be the
   release version.
   * If there is _merge from a hotfix branch_, then we increment the 
     _right most digit_ of the version.
   * If there is _merge from the develop branch_, then we increment the
     _middle_ digit of the version.
2. Every merge on the develop branch from a feature branch then must have a 
   tag, and this will be the bulid version
