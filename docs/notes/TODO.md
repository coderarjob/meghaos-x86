# MEGHA OPERATING SYSTEM V2 - x86
----------------------------------------------------------------------------------------------------

## THINGS TO DO

### TASKS/BUGS

|-------------------------------------------------------------------------|-----------|------------|
| Tasks                                                                   | Date      | Date       |
|                                                                         | Added     | Completion |
|-------------------------------------------------------------------------|-----------|------------|
| TESTING                                                                 |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [X] unittest.h bug:                                                     |           | 20 JUL 23  |
| Function under test is called twice when a scalar match fails.          |           |            |
| [X] Utils unittest                                                      | 09 JAN 24 | 09 JAN 24  |
| [X] libcm unittest                                                      | Nov 24    | 21 Nov 24  |
|-------------------------------------------------------------------------|-----------|------------|
| DOCUMENTATION                                                           |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Document BOOTINFO struct                                            |           |            |
| [ ] There is a need to document the types declared for MeghaOS like     |           |            |
| INT, LONG etc. These do need to conform with C99, but can be different  |           |            |
| from the compiler versions. LONG for example must be 32 bits or more,   |           |            |
| but always large enough to hold the largest addresses in the            |           |            |
| architecture.                                                           |           |            |
| [X] Physical Map bug: Every region must end at a page boundary.         | 29 Dec 23 | 27 Feb 24  |
| Free region starting 0x90000 ends at 0x9FBFF. PAB keeps track of pages, |           |            |
| it cannot track page half used.                                         |           |            |
| [[ ]] Kernel IDT cannot be at 0x00000. It contains the real mode IVT    | 29 Dec 23 |            |
| must not be overriden.                                                  |           |            |
| [X] Document TSS for later                                              |           | 28 JUN 21  |
| [X] Document IDT for later                                              |           | 28 JUN 21  |
| [X] Document Call Gates                                                 |           | 28 JUN 21  |
| [X] Document Paging                                                     |           | 24 JUL 21  |
| [X] Header file structures                                              |           |            |
| [X] Kmalloc documentation                                               | 17 Mar 24 | 20 Mar 24  |
|-------------------------------------------------------------------------|-----------|------------|
| BUILD SYSTEM                                                            |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Separate warning flags for Release, and user processes              | 12 Apr 24 |            |
| [ ] GCC-10.5 adds -Wenum-conversion, which will help typecheck enum     | 04 Feb 24 |            |
| variants in C. Does nothing if flags are OR'ed.                         |           |            |
| [ ] Add -Wundedf compiler flags                                         |           |            |
| [ ] Remove libgcc. Write two macros do_div() for div & mod opeartions   | 06 Mar 24 |            |
| [ ] Configuration header files for C and assembly.                      |           |            |
| Note: This can be done using CMake.                                     |           |            |
| [ ] Script to auto generate the config headers simultaneously for       |           |            |
| C and assembly.                                                         |           |            |
| NOTE: This can be done using CMake, so separate script is not required. |           |            |
| [ ] Using Flawfinder to check for security flaws seem intersting.       |           |            |
| [X] Switch to CMake from build.sh                                       |           | 25 May 24  |
| ::: [ ] Its confusing to have targets like mpdemo, mpdemo.flt.in &      | 24 Jul 24 |            |
| ::: ::: mpdemo.flt and also boot0.flt-lib & boot0.flt. Why '-lib' and   |           |            |
| ::: ::: not '.in' for intermediate files?                               |           |            |
| [X] x86 Cross compiler setup                                            |           |            |
| [X] Create a tools folder, with scripts to build gcc cross-compiler.    |           |            |
| [X] Idea of VERBOSE to print debug messages on screen is faulty.        |           | 07 SEP 21  |
| There should be options like:                                           |           |            |
| DEBUG_ONSCREEN,                                                         |           |            |
| DEBUG_E9,                                                               |           |            |
| DEBUG_SERIAL,                                                           |           |            |
| DEBUG_FILE.                                                             |           |            |
| Correct option is selected based on compiler flags and stage at which   |           |            |
| the OS is currently.                                                    |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| CODE STYLE                                                              |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Need to rethink where PTR and void* should be used. Being in the    | 16 Apr 24 |            |
| Kernel it makes this distinction less obvious, but some thought is      |           |            |
| requried.                                                               |           |            |
| [X] Functions should return EXIT_SUCCESS, EXIT_FAILURE on               |           | 23 NOV 22  |
| Success and Failure. Not ERR_NONE, on Success, and -1 on failure.       |           |            |
| Issue first seen in kdisp module.                                       |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [X] Create/Update the C style guide.                                    |           | 29 DEC 21  |
| ::: [-] Instead of just [ER], boot1 should show some error code, that   |           |            |
| identifies the failure.                                                 |           |            |
| NOTE: Not done, because there is currently only one souce of error in   |           |            |
| each of the routimes. Specific error codes will not be benifitial at    |           |            |
| this time.                                                              |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Make symbols comply with the C99.                                   |           |            |
| ::: [ ] Symbols beginning with underscore are reserved [7.13]. Rename.  |           |            |
| ::: [X] Macros in header files.                                         |           |            |
| ::: [X] Variable names in C source.                                     |           |            |
| ::: [X] Members in struct.                                              |           |            |
| ::: [X] Function names.                                                 |           |            |
| ::: [ ] Guard Macros in header files.                                   |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [X] Use true, false from stdbool.h instead of macros defined in         |           | 04 APR 23  |
| our types.h                                                             |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Spaces before paramter need to be rethought.                        | 04 Feb 24 |            |
| May be `ControlStatementsExceptControlMacros` is better than            |           |            |
| `NonEmptyParentheses`                                                   |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| DEVICE DRIVERS                                                          |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] PS2 interface is little bit noisy and needs cleanup. I think the    | 12 Sep 24 |            |
| interface is illogical and needs some thought.                          |           |            |
| [ ] Keyboard driver hangs soon after responding to one or two key       | 12 Sep 24 |            |
| strokes. Works fine in VBox, 89Box, Qemu and Bochs.                     |           |            |
| [ ] k_delay produces inconsistent delays between Qemu, 86Box & Bochs    | 23 Oct 24 |            |
| It seems to be working properly on HW. Working in Bochs                 |           |            |
| after 'clock: realtime' added to bochsrc. Still improper in 86Box.      |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| GRAPHICS                                                                |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [X] kearly_printf, kdebug_printf should be debug specific funcitons.    | 16 Nov 24 | 17 Nov 24  |
| Implementation:                                                         |           |            |
| * `kearly_printf` is now a DEBUG specific function.                     |           |            |
| * `kearly_printf` would print to VGA Text buffer in text mode. In       |           |            |
| graphics mode it does nothing.                                          |           |            |
| * `kdebug_printf` is removed.                                           |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [X] Logs in graphics mode should not print to VGA text buffer           | 13 Nov 24 | 17 Nov 24  |
|-------------------------------------------------------------------------|-----------|------------|
| [X] k_panic in graphics mode - Need to have very low dependency. May be | 13 Nov 24 | 17 Nov 24  |
| have to draw directly to video frame buffer                             |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| LIBC MOS                                                                |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [X] APPLIB is just a terrible name.                                     | 13 Nov 24 | 21 Nov 24  |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Process events need to be handled such that no event gets lost.     | 13 Nov 24 |            |
| Sort of what Windows                                                    |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| KERNEL                                                                  |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Determine CPU/machine capabilites and take steps if they are less   | 17 Aug 24 |            |
| than the minimum required. Here are some of the items we requrire:      |           |            |
| * P6 processor                                                          |           |            |
| * Protected mode                                                        |           |            |
| * Paging support                                                        |           |            |
| * FPU installed                                                         |           |            |
| * 800x600, 8bpp VESA mode                                               |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] How to determine if an FPU is installed and how to enable it if     | 17 Aug 24 |            |
| it is.                                                                  |           |            |
| ::: [ ] Print floating point (double & float) using kprintf             | 17 Aug 24 |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Are assertOnError and panicOnError requried? They check if errno    | 16 Apr 24 |            |
| is set then one asserts and other panics. However as errno is never     |           |            |
| reset, these macros can only be used when a function fails, so the      |           |            |
| check they do internally is not really required. Moreover assertOnError |           |            |
| macro makes little sense, why you want to handle an error with an       |           |            |
| assert and when to panic just call panic why panicOnError is required.  |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [X] BUG: It is possible to write to a readonly page                     | 12 Apr 24 | 13 Apr 24  |
| Reason:                                                                 |           |            |
| This happened because the CR0.WP (Write Protect) flag was unset.        |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [X] Since locations for PAB, initial PD and PT are fixed, the need for  | 17 Jul 24 | 19 Jul 14  |
| ::: g_pab, g_page_dir, g_page_table are not required. Only issue is     |           |            |
| ::: unittests how are we going to mock, so for unittests something else |           |            |
| ::: need to be done.                                                    |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Process management                                                  | 11 Apr 24 |            |
| ::: [X] Process Killing: Need a way to pass exit code to parent process | 21 Apr 24 | 11 Nov 24  |
| Possible solution.                                                      |           |            |
| When a process ending it would add SIGCHILD signal for its parent       | 21 Apr 24 |            |
| and the scheduler will make sure that the parent gets the message.      |           |            |
| Need a way to avoid creation of ZOMBIE processes.                       |           |            |
| ::: [ ] Process Exit: Copy the Kernel PDEs from PD of the process to    | 27 Apr 24 |            |
| ::: PD of the Kernel. At the start we copy the Kernel PDEs but are not  |           |            |
| ::: copying it back. The Kernel memory mapping might have changed by    |           |            |
| ::: now, so this copying is needed.                                     |           |            |
| ::: [X] Process hierarchy. New processes will have the current process  | 06 Nov 24 | 06 Nov 24  |
| as its parent.                                                          |           |            |
| ::: [X] Process Kill: When killing a process, its child processes are   | 06 Nov 24 | 06 Nov 24  |
| also killed.                                                            |           |            |
| ::: [X] Free up process memory when creation or switching fails         | 15 Apr 24 | 10 Aug 24  |
| ::: [-] Ability to select and extend stack memory as required.          | 12 Apr 24 | 03 Aug 24  |
| With VMM the stack can be allocated dynamically but is of fixed size,   |           |            |
| but since we now have lazy on-demand page allocation, the fixed size    |           |            |
| can be large and does not require explicit 'extention'.                 |           |            |
| ::: [X] Process exit: Free both virtual physical memory at process exit | 18 Jul 24 | 03 Aug 24  |
| ::: [X] Process creation/killing: Binary and stack size of only 4KB is  | 21 Apr 24 | 03 Aug 24  |
| supported.                                                              |           |            |
| ::: [X] Process Killing: Free Page tables along with page directory     | 21 Apr 24 | 21 Apr 24  |
| ::: ::: [X] Cannot unmap stack for Kernel process/threads because the   | 22 Apr 24 | 23 Apr 24  |
| ::: ::: stack is used in the Kernel mode. kpg_unmap will cause page     |           |            |
| ::: ::: fault when it tries to access the stack or return.              |           |            |
| ::: [X] Process temporary map seems ad-hoc solution. Requries polish    | 12 Apr 24 | 22 Apr 24  |
| ::: [X] InterruptFrame structure requires to take into account          | 14 Apr 24 | 18 Apr 24  |
| that for Kernel processes, SS:ESP will not be passed.                   |           |            |
| ::: [X] Queue for the scheduler                                         |           | 21 Apr 24  |
| ::: [X] Coorporative multitasking setup                                 |           |            |
| ::: ::: [X] Examine the behaviour of system call                        | 14 Apr 24 | 15 Apr 24  |
| ::: ::: [-] First Come First Serve (FCFS) for process scheduling needs  | 14 Apr 24 | 21 Apr 24  |
| thought.                                                                |           |            |
| The scheduling policy is not FCFS, but "Earliest Process Idle First".   |           |            |
| Purely FCFS can only be used in batch/single process systems.           |           |            |
| ::: [X] System call for exiting process (thread or otherwise)           |           | 21 Apr 24  |
| ::: [X] yield syscall                                                   |           | 15 Apr 24  |
| ::: [-] BUG: Page fault when user thread creates new process            | 11 Apr 24 | 12 Apr 24  |
| Does not happend when a kernel thread in creating the process.          |           |            |
| Solution: This was not a bug, but expected. Creation of process         |           |            |
| requires access to Kernel pages (kmalloc) which user mode cannot do.    |           |            |
| Moreover, kpg_map invalidates caches, which are privilaged instructions |           |            |
| ::: [X] BUG: 86Box and Bochs resets when making syscall from process1   | 11 Apr 24 | 12 Apr 24  |
|-------------------------------------------------------------------------|-----------|------------|
| [X] Rename salloc to ksalloc.                                           | 16 Mar 24 | 03 Aug 24  |
|-------------------------------------------------------------------------|-----------|------------|
| [X] Rename include/assert.h to kassert.h.                               | 16 Mar 24 | 17 Mar 24  |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Few utils macros like MIN, MAX are unsafe. We could use GNU         | 04 Feb 24 |            |
| `statement-expressions` to mitigate these.                              |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] BIT_SET, BIT_CLEAR macros are too limiting. They are too verbose    | 12 Sep 24 |            |
| compared to how much they do. They could be benifitial if we could      |           |            |
| pass a bit mask telling all the bits that need setting and clearing     |           |            |
| in one go.                                                              |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] String functions like strcmp etc                                    | 06 Mar 24 |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Have two macros like BUG_ON, WARN_ON BUG_ON panics, WARN_ON         |           |            |
| prints warning. They will replace k_assert and remain in release        |           |            |
| as well, thus these macros must have little impact on binary size       |           |            |
| and loose the text description of k_assert.                             |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [X] Remove k_errorText. Has no use in the kernel. It just wasts space.  |           | 24 Mar 24  |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Space saving measures                                               |           |            |
| ::: [X] \n instead of \r\n. No use of LF.                               |           | 16 Mar 24  |
| Did nothing for space saving for some reason.                           |           |            |
| ::: [X] 0x printed automatically with %x.                               |           | 24 Apr 24  |
| ::: [ ] Assert message only valid for unittests.                        |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [X] Create a Kernel context Structure. It will contain various          |           | 05 Apr 24  |
| global pointers state, etc required by the kernel.                      |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Virutal memory allocator                                            |           |            |
| ::: [X] Lazy allocation                                                 |           | 22 Jul 24  |
| ::: [X] Allocate a new virtual address                                  |           | 25 Mar 24  |
| ::: [X] Map a virtual to physical address                               |           |            |
| ::: [[X]] Higher half mapping maps first 2MiB physical memory           |           | 23 Mar 24  |
| entirely to 0xC0000000. Initially only the physical memory where the    |           |            |
| Kernel binary resides need to be higher-half mapped. Rest should go     |           |            |
| through PMM and VMM/Paging.                                             | 03 Mar 24 |            |
| ::: [ ] May be kpg_temporaryMap/Unmap can call kpg_map.                 | 14 Jul 24 |            |
| ::: [X] kvmm_memmap API may simplify the VMM API.                       | 17 Aug 24 | 18 Aug 24  |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] DEBUG_LEVEL determines where debug messages are printed. Does it    | 27 May 24 |            |
| makes sense to print debug messages in NDEBUG build mode?               |           |            |
| [X] Log and print to E9 port the following:                             |           | 06 Mar 24  |
| ::: [X] Function name & location when entering and leaving functions    |           | 06 Mar 24  |
| FUNC_ENTRY macro can be used for this. Not doing Logging when leaving.  |           |            |
| ::: [X] Just before change to PT/PD etc.                                | 26 FEB 24 | 06 Mar 24  |
| Can be acheived using the INFO macro.                                   |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [X] Println and Print instead of printf                                 | 03 Mar 24 | 06 Mar 24  |
|-------------------------------------------------------------------------|-----------|------------|
| [X] Physical memory allocator                                           |           | 20 JUL 23  |
| ::: [X] Reimplement using common/bitmap                                 |           |            |
| ::: [ ] Lock to protect the PAB.                                        |           |            |
| ::: [ ] Make allocation and deallocation atomic                         |           |            |
| ::: [X] PAGEFRAMES_TO_BYTES seems duplicate of PAGEFRAME_TO_PHYSICAL    | 4 JAN 24  | 09 JAN 24  |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Various functions like kdisp_ioctl, etc must implement error        |           |            |
| reporting.                                                              |           |            |
| ::: [X] kdisp_ioctl                                                     |           |            |
| NOTE: Afterthought: Is this a good idea?                                |           |            |
| Calls or or any of its parameters to these functions come not           |           |            |
| from a human, but from other parts of the OS, any wrong                 |           |            |
| argument getting to these functions means there is something wrong      |           |            |
| within the kernel code. We should fail fast the moment such conditions  |           |            |
| occur in the kernel.                                                    |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [X] k_panic called from interrupt handlers must show correct            |           | 08 JUL 23  |
| exception location. Use the interrupt frame.                            |           |            |
| [X] Print interrupt frame in exception handlers                         |           | 08 JUL 23  |
| [X] Page fault handler in asm                                           |           | 22 Mar 23  |
| ::: [-] Examine if EFLAGS register need to be preserved by interrupt    |           |            |
| and exception handlers.                                                 |           |            |
| [X] Remove the transparent union for PHYSICAL                           |           | 19 NOV 22  |
| Type matching must be rigid to be useful.                               |           |            |
| [X] GP handler                                                          |           | 22 MAR 23  |
| [X] Rename the accessor methods in boot.c to                            |           | 11 NOV 22  |
| the following format: k<Boot Structure Name>_get<Field Name>            |           |            |
| [X] GDT and LDT concept                                                 |           |            |
| [X] IDT and TSS read                                                    |           |            |
| [X] Test Protected mode entry                                           |           |            |
| [X] GDT.c                                                               |           |            |
| [X] VGA text driver                                                     |           |            |
| [X] mem.c                                                               |           |            |
| [X] TSS                                                                 |           |            |
| [X] Jump to ring 3 (demo)                                               |           |            |
| [X] IDT (demo)                                                          |           |            |
| [X] System calls demo (demo)                                            |           |            |
| [X] DIV0 handler                                                        |           |            |
| [X] 32Bit Paging                                                        |           |            |
| [X] Identity Paging (demo)                                              |           |            |
| [X] Page fault handler (demo)                                           |           |            |
| [X] A way to pass information from bootloader to the kernel             |           |            |
| [X] Memory detection                                                    |           |            |
| [X] Check minumum memory in boo1                                        |           |            |
| [X] Higher half kernel implement                                        |           |            |
| [X] BOOT1 to load Kernel and INITRD.0                                   |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| OBSOLETE                                                                |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [X] Treat physical page 0 as a NULL page.                               |           | 19 NOV 22  |
| PMM will never allocate this page and thus no Virtual Memory can        |           |            |
| be assigned to this page. This physical address will be treated         |           |            |
| as an invalid (out of range) address by both PMM and VMM.               |           |            |
| So can be helpful to track down bugs, as any access to this address,    |           |            |
| means something is not properly initialized. Without this NULL page,    |           |            |
| such errors are hard to track down.                                     |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| LOW PRIORITY                                                            |           |            |
|-------------------------------------------------------------------------|-----------|------------|
| [ ] Basic keyboard IRQ handler (demo)                                   |           |            |
| [ ] RAMDISK (INITRD.0) creation in build.sh                             |           |            |
| [-] Check the effect on kernel binary size, if the boot opaque types    |           |            |
| were removed. PMM or the Kernel accesses fields directly.               |           |            |
| I think these opaque types should remain, the accessor methods are a    |           |            |
| good place to place asserts and other checks. And these checks are      |           |            |
| invaluable in finding and preventing bugs.                              |           |            |
| [ ] `LoadFile` routine in `boot0` to read from FAT32 not FAT12.         |           |            |
| This is needed becuase UEFI boot disks must be formated in FAT32.       |           |            |
| [ ] (Optional) A seek(..) type routine, so that we can load different   |           |            |
| parts of file to memory. This comes handy to load the INITRD            |           |            |
| (which could be > 64 KB).                                               |           |            |
|                                                                         |           |            |
| This is optional becuase we can live without this routine, if we split  |           |            |
| our INTRD in two parts. 1st part is smallest, which is loaded by        |           |            |
| `boo1`, it contains the kernel and rest of the initial part of the OS.  |           |            |
| The rest will be lumped together in the 2nd INITRD file.                |           |            |
|                                                                         |           |            |
| NOTE: Seek need not be a separate routine, we can just implement it     |           |            |
| inside the `LoadFile` routine, and take a `start` location as           |           |            |
| argument.                                                               |           |            |
|-------------------------------------------------------------------------|-----------|------------|

### NOTES

|-------------------------------------------------------------------------------------------------|
| NOTES                                                                                           |
|-------------------------------------------------------------------------------------------------|
| [X] Can kernel global variables like errno, etc, lie in the kernel, or be placed in the process |
| struct, so that re-entrant kernel problems do not occour.                                       |
| - Each process has separate kernel data and kernel stack.                                       |
| Thus no conflict, between processes.                                                            |
|-------------------------------------------------------------------------------------------------|
