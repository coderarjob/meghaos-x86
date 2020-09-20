## Currently in development: v2.0.0-x86-dev

### Date: 4th Sep 2020
1. Folder restructure with the following tree
```
	.
	├── bootloader
	│   └── x86
	│       ├── phase1
	│       └── phase2
	├── build
	│   ├── diskimage
	│   │   └── x86
	│   ├── list
	│   ├── obj
	│   ├── sym
	│   └── temp
	├── docs
	│   └── x86
	├── drivers
	│   └── x86
	│       ├── 8259
	│       └── keyboard
	├── include
	│   ├── asm
	│   └── x86
	│       └── asm
	├── kernel
	│   └── x86
	├── tools
	└── util
```
2. Git Branching and Versioning convention is added ot the README.md file.
3. New build.sh file.
4. Loader.ld linker script for the 2nd Stage loader added to source tree.

### Date: 18th Sep 2020
1. Bootloader program renamed from `boot.flt` to `boot0.flt`
2. 2nd Stage Loader program renamed from `loader.flt`/`LOADER` to `boot1.flt`
3. File names that appear in `obj` folder is the one that will be copied to the
   floppy image exaxtly. No renaming.
4. `build.sh` file deletes files from the above tree at the very beginning.
5. `boot1` program shows a welcome message with `OS_NAME` and build versions.
6. Debug/Status messages will be in the following format: </br>
   ```
    MESSAGE: module space space status+
    module : <some text that identifies the routine getting called>
    status : OK|!
    space  : hex 20
   ```
   No Sophistication in text. Messages should be plain and simple, with no
   extra.
7. A20 methods `using BIOS` and `fast A20` implemented and tested.

### Date: 20th Sep 2020
1. `boot1` setups the GDT and enters protected mode.
2. `boot1` also loads kernel module at location 0x1800:0x0000.
3. Jumps to kernel.
4. Test Kernel starts completely in 32 bit protected mode with CPL = 0. It is
   completely written in C.
5. GCC16 is no longer required as no real mode part of the OS is written in C.
6. Added a new kernel.ld file.
