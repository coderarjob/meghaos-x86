# Megha Operating System V2
------------------------------------------------------------------------------

##  Different ways to handle error/invalid input in Kernel
categories: note, independent
_25 June 2023_

Put assertions when you know that the input/state of a variable must be valid. You assume this
because, it should have been validated before reaching to this place. It must be an impossibility
for an assertion to fail - but when it does it means that there is a bug.

To be sure that hitting an assert is an impossibility, there must be checks before. This presents
two problem:
1. At some point something must do the check and not assert. What should be that place?
2. When writing an OS from the bottom, there is no visibility who is supposed to check these inputs.
   Which is again a case for my confusion.

If we think every data belongs to a module, then top-level functions of that module should check for
their validity. It reports back the error and sets appropriate error code.

What I mean by top level depends what your module contains:
1. Single file module: Top level functions are the public functions.
2. Multi file modules: Top level functions are functions which directly get input from other
   modules.

For example:
A) If common/bitmap.c is thought of one module then, top-level functions are `bitmap_*` public
   functions. They must validate the input and report back if validation fails.

B) If we say kernel/pmm.c and common/bitmap.c make one module, then top-level functions are `pmm_*`
   public functions, because no other part of the OS will call `bitmap_*` functions directly for PMM
   operations. In this case `pmm_*` functions should validate and bitmap functions can assert.

Note that failing an assert should not mean calling panic and halting the system. There can be
conditions where a warning can be enough. If an assert failure is not fatal, print a warning.

------------------------------------------------------------------------------

## When to panic in a kernel?
categories: note, independent
_28 June 2023_

Clearly when it is unsafe to continue, only then we panic. Situations when continuing may cause
unexpected behaviour. In all other cases we do not panic.

If resource cannot be allocated for one process, does not mean to bring the system down. It there's
a file which cannot be read from memory, it does not make sense to bring the system down. In these
and other situations where OS can continue working, it should - print out a detailed warning and
continue.

So in most cases, we must continue and not panic.

There should be two macros `Panic` and `Warn` together with two assert macros `Assert_Fatal` and
`Assert_Warn` respectively.
