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

