# MEGHA OPERATING SYSTEM V2 - x86

A general purpose x86 Operating System that runs of a 1.44 MiB floppy and requires 1.44 MiB of RAM.

MeghaOS is completely written from scratch and primarily a learning project. The goal for MeghaOS is
to provide an environment to do experiments with the computer and to play around.

Think of MeghaOS as a bike with training wheels - there is protection, but they can be disabled or
changed by the rider.

I want the system to be stable but not totally locked down to the programmer. The OS will run in
x86 Protected Mode with Virtual Memory to ensure that one process do not touch memory used by
another process. The kernel will provide ways for the programmer to interact with the computer
safely and change parts of the kernel easily.

![MeghaOS Screenshot](/docs/images/meghaos_screenshot.png)

This is the second iteration of an operating system that I am building. The first was a Real Mode
OS, intended to run on a 8086 processor. It was written entirely in assembly language, because of
the lack of proper higher level language compilers for the 8086 processor.

The current, second version, targets the 80386 processor and the former unavailability of tools is
no longer there, and is thus most written in C.

PS: OpenWatcom is a great C compiler if you want to target 8086. However, I wanted to stick with
something more standard and more common.

The end product will be ready for a programmer but not for general use.

## Roadmap

- [X] 1st stage bootloader installing FAT12 Real mode routines and loading 2nd stage loader.
- [X] 2nd stage bootloader loading the kernel and other modules.
- [X] GDT and TSS setup enter to protected mode.
- [X] Higher-Half page mapping
- [X] Jumping to User mode (Privilege Level 3) from Kernel mode.
- [ ] Interrupt handlers for CPU Exceptions: GPF and PF handlers working. **(incomplete)**
- [X] Basic Kernel mode C functions for printing on screen etc.
- [X] Unittesting framework to test parts of the OS on host computer.
- [ ] Memory management: Physical page allocation. **(developing)**
- [ ] Memory management: Virtual page allocation.
- [ ] CPIO based RAMDISK FS, for loading kernel modules and other programs.
- [ ] User mode processes

## Design goals

1. Small modular kernel
2. Modules and drivers can be selected and loaded during booting. As and when needed, any modules
   can be replaced or disabled.
3. FAT12 filesystem.
6. Will target x86 Protected Mode, but can be ported easily to other architectures.
7. VFS, DEVFS and CPIO based RAM Disk file system.
8. MeghaOS is not going to use too much x86 specific features, because that will make porting
   difficult.

## Building MeghaOS

After the below perquisites are met, just run `./build.sh`. This will build the floppy image, as
well as the unittests.

### Prerequisites

1. Requires an linux environment. May also be possible on WSL.
1. gcc and binutils version 8.3 of higher configured to target 1686-elf.
   Use `tools/build_i686_gcc.sh` to configure and install. Add the installation path to the $PATH
   variable.
2. nasm assembler version 2.15.05 or higher.

### Prerequisites: Unittests

1. gcc and binutils 8.3 or higher.
2. gcc-multilib if host computer processor is anything other than x86.

## Running on host computer.

Can be run on natively on any x86, x86\_64 computer. Flash a pendrive with the floppy image and
boot from it.

You can also run it on on an emulator like Qemu for VirtualBox.
If you have Qemu, just run `./run.sh`.

To run the unittests do run `./run.sh unittests`.

## Development Specifics

### Git Branches

The `master` branch have the latest changes. Merges from a feature or hotfix branches all go into
the `master` branch. After a significant milestone, I will tag a commit, so you can check those
if you do not want the very latest, but do not know why you would not.

* Master  - Current development branch where all the feature and hotfix branches merge into.
* Feature - Lives temporarily for a feature or non specific development.
* Hotfix  - Lives temporarily and named like `hotfix-ls-segfault`.

```
HotFix   Master       Feature
  |        |           |
  |        |           |
  |        |           |
  |<------>|<--------->|
```

### Semantic Versioning Scheme:

1. Version will be structured:   
    `major.minor.patch-build.-releasetype`

   Example: `1.2.19-200909.1-dev`

2. `build` is in the format: `<year><month><day>.<build_minor>`.

3. `releasetype` are : `dev`, `alpha`.

4. |Version| Reason for change                                     |
   |-------|-------------------------------------------------------|
   |Major  | Increments when backward compatibility is broken.     |
   |Minor  | Increments when backward compatibility is maintained. |
   |Patch  | Bug fixes, that does not break backward compatibility.|

5. Whenever the left or middle digit changes, the digits to the right is reset to zero.
   * 1.2.14  -->  1.3.0  --> 2.0.0
