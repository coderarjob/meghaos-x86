## Megha Operating System - V2
--------------------------------------------------------------------------------

**Run this command to refresh documents list: `bash collate.sh`**

### TODO
- [TODO](notes/TODO.md)
- [Scratch](notes/Scratch.md)
- [BUGS](notes/BUGS.md)

### Guidelines Docs
- [Unittests file system hierarchy in MeghaOS](notes/source_tree_style.md#Unittests-file-system-hierarchy-in-MeghaOS)
- [Build scripts in MeghaOS](notes/source_tree_style.md#Build-scripts-in-MeghaOS)
- [Cross platform guidelines](notes/CrossPlatform.md#Cross-platform-guidelines)
- [MeghaOS code style](notes/coding_style.md#MeghaOS-code-style)

### Obsolete Guidelines Docs
- [Source tree conventions](notes/source_tree_style.md#Source-tree-conventions)

### x86 Guidelines Docs


### Obsolete x86 Guidelines Docs

------------------------------------------------
### Features Docs
- [Kmalloc working](notes/kmalloc.md#Kmalloc-working)
- [Process management](notes/processman.md#Process-management)

### Obsolete Features Docs


### x86 Features Docs
- [Physical memory map](notes/gdt.md#Physical-memory-map)
- [VESA VBE Graphics mode](notes/Vbe.md#VESA-VBE-Graphics-mode)
- [Physical Memory allocation](notes/physical_memory_manager.md#Physical-Memory-allocation)
- [Operating System ABI](notes/abi.md#Operating-System-ABI)
- [Interrupt handling](notes/interrupt_handling.md#Interrupt-handling)
- [Virtual memory map](notes/vmm.md#Virtual-memory-map)

### Obsolete x86 Features Docs


------------------------------------------------
### Notes
- [Understanding when to use assert in Kernel code](notes/when-to-assert.md#Understanding-when-to-use-assert-in-Kernel-code)
- [When to panic in a kernel?](notes/when-to-assert.md#When-to-panic-in-a-kernel)
- [Abstraction is Key for a modular Operating System](notes/abstract_design.md#Abstraction-is-Key-for-a-modular-Operating-System)
- [Is it required for all the structures to be opaque?](notes/opaque_data_types.md#Is-it-required-for-all-the-structures-to-be-opaque)
- [Why are opaque data types needed?](notes/opaque_data_types.md#Why-are-opaque-data-types-needed)
- [Assert/Panic handling in unittests](notes/assert-panic-in-meghaos.md#Assert/Panic-handling-in-unittests)
- [User events in GUI environment](notes/ui.md#User-events-in-GUI-environment)
- [To POSIX or not to POSIX](notes/general.md#To-POSIX-or-not-to-POSIX)
- [Study on executable binary file format for the Kernel and other module files.](notes/object_file_format.md#Study-on-executable-binary-file-format-for-the-Kernel-and-other-module-files.)
- [Equivalence of ELF and flat binary files before and after CMake](notes/cmake-build.md#Equivalence-of-ELF-and-flat-binary-files-before-and-after-CMake)

### Future feature ideas
- [Rethinking the idea of Files](notes/ui.md#Rethinking-the-idea-of-Files)
- [User Interface notes](notes/ui.md#User-Interface-notes)

### x86 Notes
- [Why separate stack does not work with GCC](notes/gdt.md#Why-separate-stack-does-not-work-with-GCC)
- [Stack overflow protection using GDT](notes/gdt.md#Stack-overflow-protection-using-GDT)
- [Theory of mixing platform dependent and independent parts](notes/Scratch.md#Theory-of-mixing-platform-dependent-and-independent-parts)
- [How to test stack overflow/underflow checks](notes/Scratch.md#How-to-test-stack-overflow/underflow-checks)
- [Keeping track of Virtual pages using Bitmap](notes/Scratch.md#Keeping-track-of-Virtual-pages-using-Bitmap)
- [System call frame](notes/Scratch.md#System-call-frame)
- [Round-robin operation using queue](notes/Scratch.md#Round-robin-operation-using-queue)
- [Code snippets](notes/Scratch.md#Code-snippets)
- [System call stack in User and Kernel processes](notes/Scratch.md#System-call-stack-in-User-and-Kernel-processes)
- [Different way to switch privilege levels in x86 processors](notes/Scratch.md#Different-way-to-switch-privilege-levels-in-x86-processors)
- [Strange behaviour because of Stale TLB](notes/Scratch.md#Strange-behaviour-because-of-Stale-TLB)
- [Virtual memory management - Regions and Mapping](notes/virtual_memory_manager.md#Virtual-memory-management---Regions-and-Mapping)
- [Why I am skipping Virtual Memory manager for now](notes/virtual_memory_manager.md#Why-I-am-skipping-Virtual-Memory-manager-for-now)
- [Virtual Memory Manager and Recursive mapping](notes/virtual_memory_manager.md#Virtual-Memory-Manager-and-Recursive-mapping)
- [Map virtual memory to physical memory](notes/virtual_memory_manager.md#Map-virtual-memory-to-physical-memory)
- [What happens if kernel stack is used in user mode](notes/usermode_stack.md#What-happens-if-kernel-stack-is-used-in-user-mode)
- [Where should we initiate and start using paging? ](notes/paging.md#Where-should-we-initiate-and-start-using-paging-)
- [How to use page tables to map Virtual Addresses to Physical Addresses?](notes/paging.md#How-to-use-page-tables-to-map-Virtual-Addresses-to-Physical-Addresses)
- [Notes: Why not preallocate the whole 256 MB? ](notes/paging.md#Notes:-Why-not-preallocate-the-whole-256-MB-)
- [Physical page allocation basics](notes/memorymanagement.md#Physical-page-allocation-basics)
- [Recursive mapping and temporary mapping in Kernel](notes/recursive_map_clarity_with_example.md#Recursive-mapping-and-temporary-mapping-in-Kernel)
- [System call frame and signature](notes/abi.md#System-call-frame-and-signature)

### Obsolete Notes
- [Different ways to handle error/invalid input in Kernel](notes/when-to-assert.md#Different-ways-to-handle-error/invalid-input-in-Kernel)
- [Asserts and Panics in Unittests in MeghaOS](notes/assert-panic-in-meghaos.md#Asserts-and-Panics-in-Unittests-in-MeghaOS)

### Obsolete x86 Notes
- [Physical Memory Map](notes/gdt.md#Physical-Memory-Map)
- [Where to keep the GDT?](notes/gdt.md#Where-to-keep-the-GDT)
- [Virtual memory management - Concept of Regions](notes/virtual_memory_manager.md#Virtual-memory-management---Concept-of-Regions)
- [Physical Memory allocation - Making physical address 0x0000 invalid](notes/physical_memory_allocation.md#Physical-Memory-allocation---Making-physical-address-0x0000-invalid)
- [Physical Memory allocation - Dry run](notes/physical_memory_allocation.md#Physical-Memory-allocation---Dry-run)
- [Physical Memory allocation - Notes on API](notes/physical_memory_allocation.md#Physical-Memory-allocation---Notes-on-API)
- [Physical Memory allocation - Size of PAB in MeghaOS](notes/physical_memory_allocation.md#Physical-Memory-allocation---Size-of-PAB-in-MeghaOS)
