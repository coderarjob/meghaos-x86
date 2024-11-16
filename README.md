# MEGHA OPERATING SYSTEM V2 - x86

32-bit, graphical, multitasking, operating system for Intel i686 (Pentium) and later processors.

![Unittest build status](https://github.com/coderarjob/meghaos-x86/actions/workflows/utbuild.yaml/badge.svg)  ![MOS build status](https://github.com/coderarjob/meghaos-x86/actions/workflows/osbuild.yaml/badge.svg)

## ABOUT

MeghaOS is written completely from scratch and is primarily a educational project. It does not
adhere to any particular operating system or philosophy, but includes ideas old and new from many
sources.

I want a system that is stable yet not totally locked down to the programmer. I want to give users
the ability to play around and explore their computers.

## Screenshots

![MeghaOS Screenshot](/docs/images/meghaos_vesafb.png)

I am working to make MeghaOS a completely graphical operating system and text mode will only be for
debugging and development. It can run variety of graphics modes and supports 8-bit, 24 & 32-bit
color.

![MeghaOS Screenshot](/docs/images/meghaos_mpdemo.gif)

Multitasking capabilities using Cooperative multitasking allows processes and threads to run
simultaneously. The choice of Cooperative multitasking was intentional as it provides a good base
for development of other multitasking features later and moreover its simple, requiring little to no
synchronization between threads and processes.

The OS runs in x86 Protected Mode with Virtual Memory to ensure that one process do not touch memory
used by another.

![MeghaOS Screenshot](/docs/images/meghaos_screenshot.png)

## Roadmap

- [X] 1st stage bootloader installing FAT12 Real mode routines and loading 2nd stage loader.
- [X] 2nd stage bootloader loading the kernel and other modules.
- [X] GDT and TSS setup and entering protected mode.
- [X] Higher-Half page mapping.
- [X] Jumping to User mode (Privilege Level 3) from Kernel mode.
- [X] Handlers for CPU Exceptions and basic interrupts.
- [X] Basic Kernel mode C functions for printing on screen etc.
- [X] Unittesting framework to test parts of the OS on host computer.
- [X] Memory management: Physical page allocation.
- [ ] Memory management: Virtual page allocation. [*90% complete*]
- [X] Memory management: Basic paging operations (map/unmap etc)
- [X] Memory management: Kernel allocators (Static and Heap allocators)
- [X] Processes and threads capable of doing system calls.
- [X] Cooperative multitasking scheduling and basic process management.
- [X] Enhancements to the process management.
- [X] VESA VGA frame buffer.
- [X] Basic graphics & fonts library
- [ ] Basic drivers (Keyboard, Mouse, RTC)
- [ ] CPIO based RAMDISK FS, for loading kernel modules and other programs.
- [ ] Graphical shell.

## Building and running MeghaOS

### Prerequisites

1. Requires Linux environment. May also be possible on WSL.
2. gcc and binutils version 8.3 or higher, configured to target 1686-elf.
   Use `tools/build_i686_gcc.sh` to configure and install gcc and binutils. Follow the following
   steps:

   * Install dependencies to build GCC and binutils:  `tools/build_i686_gcc.sh --install-dep`.
   * Build and install GCC and binutils: `tools/build_i686_gcc.sh`.

   Then either add the installation path to the $PATH variable or pass the path in
   `CMAKE_PREFIX_PATH`.
3. nasm assembler version 2.15.05 or higher.
4. Cmake version >= 3.15
5. `dosfstools` for creation of disk image.

### Building

#### Cmake build options

* `CMAKE_TOOLCHAIN_FILE` (Required) - Path to toolchain file.
* `MOS_BUILD_MODE` (Defaults to DEBUG) - Valid values are DEBUG, NDEBUG.
* `MOS_PORT_E9_ENABLED` (Defaults to 0) - Enables/disables debug printing using port 0xE9.
* `CMAKE_PREFIX_PATH` - Path to where cross compiler is installed. Required if PATH environment
    variable does not include it.

Generate the build system and then start the build:
```
# Option 1: DEBUG build and cross compiler installation path passed explicitly.

$ cmake -DCMAKE_TOOLCHAIN_FILE=./tools/toolchain-i686-elf-pc.cmake \
        -DCMAKE_PREFIX_PATH=~/.local/opt/i686-cross                \
        -B build-os

# Option 2: NDEBUG mode build with MOS_PORT_E9_ENABLED = 1.

# NOTE; Cross compiler path taken from $PATH.
# NOTE: MOS_PORT_E9_ENABLED will enable printing debug messages to host terminal.

$ cmake -DCMAKE_TOOLCHAIN_FILE=./tools/toolchain-i686-elf-pc.cmake \
        -DMOS_BUILD_MODE=NDEBUG -DMOS_PORT_E9_ENABLED=true -B build-os
```
```
$ cd build-os

# Compiles the Kernel and user programs. Does not bulid disk image.
$ make

# Compiles and bulids disk image.
$ make mos.flp
```
### Running

To run the disk image in Qemu use the following command:
```
$ cd build-os

# Asuming your build system is `make`
# Builds disk image and runs in Qemu
$ make run

# Pass arguments to Qemu
$ make ARGS="<qemu arguments> run
```
## Building and running Unittests

### Prerequisites:

1. gcc and binutils 8.3 or higher.
2. `gcc-multilib` if host computer processor is anything other than x86.
3. nasm assembler version 2.15.05 or higher.
4. Cmake version >= 3.15

### Building

Build unittests using the following command:
```
$ cmake -DARCH="x86" -B build-ut
$ cd build-ut
$ make
```
### Running

To run every or any specific test use the following command:
```
$ cd build-ut

# Asuming your build system is `make`
# Run every unittests
$ make run

# Run specific test
$ make ARGS="--name <test name> run

```
### Prerequisites: Code coverage report

1. `gcov` library 8.3 or higher.
2. `lcov` and `genhtml` package.

To generate code coverage report run the following command:
```
$ cd build-ut

# Asuming your build system is `make`
$ make gen-cov
```

You will find the report in `build-ut/reports/coverage/report/index.html`.

# Feedback

Open a GitHub issue or drop a email at arjobmukherjee@gmail.com. I would love to hear your
suggestions and feedbacks.
