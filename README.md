# MEGHA OPERATING SYSTEM V2 - x86

A general purpose 32 bit Operating System for Intel i686 and later processors. It is written 
completely from scratch and is primarily a learning project. The goal for MeghaOS is to provide an 
environment to do experiments with the computer and to play around.

It runs of a 1.44 MiB floppy and can be configured to use minimal physical memory.

| ![MeghaOS Screenshot](/docs/images/meghaos_mpdemo.gif) |
|---|
| `Cooperative multitasking demo - Processes running 'simultaneously'` |

Think of MeghaOS as a bike with training wheels - there is protection, but they can be disabled or
changed by the rider.

| ![MeghaOS Screenshot](/docs/images/meghaos_screenshot.png) |
|---|
| `Stack trace when fault occurs` |

I want the system to be stable but not totally locked down to the programmer. The OS will run in
x86 Protected Mode with Virtual Memory to ensure that one process do not touch memory used by
another. The kernel will provide ways for the programmer to safely interact with the computer and 
change parts of it easily.

The end product will be ready for a programmer but not for general use.

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
- [ ] Memory management: Virtual page allocation. [*Partial*]
- [X] Memory management: Basic paging operations (map/unmap etc)
- [X] Memory management: Kernel allocators (Static and Heap allocators)
- [X] Processes and threads capable of doing system calls.
- [X] Cooperative multitasking scheduling and basic process management.
- [ ] Enhancements to the process management.
- [ ] VESA VGA driver.
- [ ] Keyboard driver.
- [ ] CPIO based RAMDISK FS, for loading kernel modules and other programs.
- [ ] Rudimentary shell.

## Building and running MeghaOS

### Prerequisites

1. Requires Linux environment. May also be possible on WSL.
2. gcc and binutils version 8.3 or higher, configured to target 1686-elf.
   Use `tools/build_i686_gcc.sh` to configure and install gcc and binutils. Follow the following
   steps:

   * Install build dependencies run:  `tools/build_i686_gcc.sh --install-dep`.
   * Build and installGCC and binutils: `tools/build_i686_gcc.sh`.

   Then either add the installation path to the $PATH variable or pass the path in
   `CMAKE_PREFIX_PATH`.
3. nasm assembler version 2.15.05 or higher.
4. Cmake version >= 3.3
5. `dosfstools` for creation of disk image.

### Building

Generate the build system and then run the build system:

```
# Option 1: DEBUG mode build
$ cmake -DCMAKE_TOOLCHAIN_FILE=./tools/toolchain-i686-elf-pc.cmake -B build-os

# Option 2: NDEBUG mode build with DEBUG_LEVEL set to print debug messages both on the screen and E9
# port.
$ cmake -DCMAKE_TOOLCHAIN_FILE=./tools/toolchain-i686-elf-pc.cmake \
        -DMOS_BUILD_MODE=NDEBUG -DMOS_DEBUG_LEVEL="3" -B build-os

# Option 2: Passing cross compiler installation path
$ cmake -DCMAKE_TOOLCHAIN_FILE=./tools/toolchain-i686-elf-pc.cmake \
        -DCMAKE_PREFIX_PATH=~/.local/opt/i686-cross -B build-os
```
```
$ cd build-os

# Compiles the Kernel and user programs. Does not bulid disk image.
$ make

# Compiles and bulid disk image.
$ make mos.flp
```
#### Cmake build options

* `CMAKE_TOOLCHAIN_FILE` (Required) - Path to toolchain file.
* `MOS_BUILD_MODE` (Defaults to DEBUG) - Valid values are DEBUG, NDEBUG.
* `MOS_DEBUG_LEVEL` (Defaults to 1) - Enables/disables debug printing. Bit 0: E9, Bit 1: Screen.
* `CMAKE_PREFIX_PATH` - Path to where cross compiler is installed. Required if PATH environment
    variable does not include it.

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
2. gcc-multilib if host computer processor is anything other than x86.
4. Cmake version >= 3.3

### Building

Build unittests using the following command:
```
$ cmake -DMOS_DEBUG_LEVEL="3" -DARCH="x86" -B build-ut
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

1. gcov library 8.3 or higher.
2. lcov and genhtml package.

To generate code coverage report run the following command:
```
$ cd build-ut

# Asuming your build system is `make`
$ make gen-cov
```

You will find the report in `build-ut/reports/coverage/report/index.html`.

## Development Specifics

Development of each part/feature is done in small increments. They are tested and refined overtime.

Stages:
1. Development starts with few ideas and a runnable prototype is made. This is a stage of heavy
   development as the core ideas are tested and refined. Initial Unittests and documentation are
   created at this stage.
2. Refinement of the software. In some cases, parts of the software may have to be rewritten.
   Further unittests and documentation are created or modified. The end product is a more stable
   runnable software.
3. Almost same as stage 2. But fewer major change is expected.

```
                   release 1    release 2    release 3
                     /            /           /
development >-------|------------|-----------|--->
                1        2            3

```

### Git Branches

The `master` branch have all the latest changes. Merges from `develop` branches go into the `master`
branch. There could also be `feature` branches which originate and are merged into the `develop`
branch.
After a significant milestone, I will tag a commit, so you can check these if you do not want the
very latest.

* Master  - Branch where all the feature and fixes merge into. Must always build.
* Develop - Majority of the development happens here. Features, bug fixing etc. May be unstable.
* Feature - Branch where side developments (which are not related work in Develop branch) occur.

```
Master       Develop     Feature
  |           |            |
  |           |            |
  |           |            |
  |<--------->|<---------->|
```

### Versioning Scheme:

1. Version will be structured: `build.releasetype.build_minor`
2. `build` is in the format: `<year><month><day>`.
3. `build_minor` starts with 0 and increments if same `build.releasetype` already exists.
4. `releasetype` are : `dev`, `alpha`.
5. Build string can be used to find the chronology of the releases.
   * 20091103.dev.0 -> 20091103.dev.1 -> 20091103.alpha.0
