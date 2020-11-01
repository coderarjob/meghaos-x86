## Megha Operating System V2 - x86
## Naming Conventions of Variables and Functions
------------------------------------------------------------------------------
_1st November 2020_

I have been following the [https://github.com/guilt/KOS][KOS], for its 
simplicity. For example routine that prints one character directly on the VGA 
buffer is named `putc`. I think this is too simple for my purpose, as the 
standard `putc` function puts characters into a file descriptor not to VGA 
buffer. We will see this pattern over and over that implementation of kernel 
function and userland functions are different. 

### Functions

* Functions that are ment to be used only in the kernel will have `k` either in
  prefix or suffix. 
* The function names generally should not tie with implementation.

  1. `kvga_putc()` is bad.
     `kdisp_putc()` is better. As it does not tie the name with VGA. What is CGA
     is used?
  2. `kmalloc()` instead of `malloc()` in kernel implementation.

* Static functions are also named normally with no indication of scope.

### Variables

* Global extern variables are named with a prefix `g_`. All external variables
  must be explicitly declared in the file that they are being used, with the
  `extern` keyword.
* Global static variables are named normally.

### Header files

* We do not want the kernel code to include too many header files all the time.
  Common header files should be included into the `kernel.h`. 
* Platform specific header files should exist inside a folder properly named. 
  **Example:** x86 `kernel.h` should be placed inside `include/x86/kernel.h`.

