## Megha Operating System - V2
--------------------------------------------------------------------------------

**Run this command to refresh documents list: `bash collate.sh`**

### TODO
- [TODO](notes/TODO.md)

### Guidelines Docs
- [MeghaOS code style](notes/coding_style.md#MeghaOS-code-style)
- [Cross platform guidelines](notes/CrossPlatform.md#Cross-platform-guidelines)
- [Unittests file system hierarchy in MeghaOS](notes/source_tree_style.md#Unittests-file-system-hierarchy-in-MeghaOS)
- [Build scripts in MeghaOS](notes/source_tree_style.md#Build-scripts-in-MeghaOS)

### Obsolete Guidelines Docs
- [Source tree conventions](notes/source_tree_style.md#Source-tree-conventions)

### x86 Guidelines Docs


### Obsolete x86 Guidelines Docs

------------------------------------------------
### Features Docs


### Obsolete Features Docs


### x86 Features Docs
- [Physical memory map](notes/gdt.md#Physical-memory-map)
- [Interrupt handling](notes/interrupt_handling.md#Interrupt-handling)
- [Physical Memory allocation](notes/physical_memory_manager.md#Physical-Memory-allocation)
- [Virtual memory map](notes/vmm.md#Virtual-memory-map)

### Obsolete x86 Features Docs


------------------------------------------------
### Notes
- [Abstraction is Key for a modular Operating System](notes/abstract_design.md#Abstraction-is-Key-for-a-modular-Operating-System)
- [Assert/Panic handling in unittests](notes/assert-panic-in-meghaos.md#Assert/Panic-handling-in-unittests)
- [To POSIX or not to POSIX](notes/general.md#To-POSIX-or-not-to-POSIX)
- [Study on executable binary file format for the Kernel and other module files.](notes/object_file_format.md#Study-on-executable-binary-file-format-for-the-Kernel-and-other-module-files.)
- [Is it required for all the structures to be opaque?](notes/opaque_data_types.md#Is-it-required-for-all-the-structures-to-be-opaque)
- [Why are opaque data types needed?](notes/opaque_data_types.md#Why-are-opaque-data-types-needed)
- [Different ways to handle error/invalid input in Kernel](notes/when-to-assert.md#Different-ways-to-handle-error/invalid-input-in-Kernel)
- [When to panic in a kernel?](notes/when-to-assert.md#When-to-panic-in-a-kernel)

### Future feature ideas
- [Rethinking the idea of Files](notes/ui.md#Rethinking-the-idea-of-Files)
- [User Interface notes](notes/ui.md#User-Interface-notes)

### x86 Notes
- [Why separate stack does not work with GCC](notes/gdt.md#Why-separate-stack-does-not-work-with-GCC)
- [Stack overflow protection using GDT](notes/gdt.md#Stack-overflow-protection-using-GDT)
- [Physical page allocation basics](notes/memorymanagement.md#Physical-page-allocation-basics)
- [Where should we initiate and start using paging? ](notes/paging.md#Where-should-we-initiate-and-start-using-paging-)
- [How to use page tables to map Virtual Addresses to Physical Addresses?](notes/paging.md#How-to-use-page-tables-to-map-Virtual-Addresses-to-Physical-Addresses)
- [Notes: Why not preallocate the whole 256 MB? ](notes/paging.md#Notes:-Why-not-preallocate-the-whole-256-MB-)
- [Recursive mapping and temporary mapping in Kernel](notes/recursive_map_clarity_with_example.md#Recursive-mapping-and-temporary-mapping-in-Kernel)
- [Why I am skipping Virtual Memory manager for now](notes/virtual_memory_manager.md#Why-I-am-skipping-Virtual-Memory-manager-for-now)
- [Virtual Memory Manager and Recursive mapping](notes/virtual_memory_manager.md#Virtual-Memory-Manager-and-Recursive-mapping)
- [Map virtual memory to physical memory](notes/virtual_memory_manager.md#Map-virtual-memory-to-physical-memory)

### Obsolete Notes
- [Asserts and Panics in Unittests in MeghaOS](notes/assert-panic-in-meghaos.md#Asserts-and-Panics-in-Unittests-in-MeghaOS)

### Obsolete x86 Notes
- [Physical Memory Map](notes/gdt.md#Physical-Memory-Map)
- [Where to keep the GDT?](notes/gdt.md#Where-to-keep-the-GDT)
- [Physical Memory allocation - Making physical address 0x0000 invalid](notes/physical_memory_allocation.md#Physical-Memory-allocation---Making-physical-address-0x0000-invalid)
- [Physical Memory allocation - Dry run](notes/physical_memory_allocation.md#Physical-Memory-allocation---Dry-run)
- [Physical Memory allocation - Notes on API](notes/physical_memory_allocation.md#Physical-Memory-allocation---Notes-on-API)
- [Physical Memory allocation - Size of PAB in MeghaOS](notes/physical_memory_allocation.md#Physical-Memory-allocation---Size-of-PAB-in-MeghaOS)
