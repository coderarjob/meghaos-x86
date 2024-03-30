# Megha Operating System V2 - x86
------------------------------------------------------------------------------

## What happens if kernel stack is used in user mode
categories: note, x86
_31 Mar 2024_

The use of kernel stack also in user mode, will cause stack corruption. The CPU sets the stack 
pointer to `Kernel Stack Top` when going from user mode to kernel mode.

Then any Push in the Kernel space will overwrite the user stack. The SS & ESP itself will be
restored when returning from an Interrupt, however the because the memory remains the common, it
gets modified by the stack operations of the Kernel mode.
