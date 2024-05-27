# Megha Operating System V2
------------------------------------------------------------------------------

##  Understanding when to use assert in Kernel code
categories: note, independent
01 May 2024

I previously said this:
> Using assert to validate input is a horrible idea. For the current implementation, failing an 
> assert freezes the machine. For an operating system this is not a good idea to halt the system 
> completely."

However I am now second guessing this. Under the normal operation kernel functions cannot receive
invalid inputs. I do not see no way for a function to receive an invalid input other than
corruption/failure somewhere. Input from processes can be wrong, and must be handled by the system 
call routines before passing them to other kernel functions.

As low level modules receive inputs from modules higher up, it asserting or returning an error code 
makes no difference because it must never have received an invalid input in the first place from the
higher modules. And an assertion check implies that such a condition is impossible to occur, which
an if check cannot convey.

For example, say a function receives a NULL pointer when it is an impossibility. If it returns a 
fault code instead of asserting then chances exist of it being ignored by the caller function and
this corruption/bug gets hidden.

The argument against asserting was this:
> If this was not the case and PMM uses assertions, then modules higher up must be checking for this
> policy violation. But who? There is no clear taker. 
> Also if this policy changes (say 0x000 address is now valid) changes will be much more spread out,
> instead of just the PMM and lower modules.

All physical address must come from one of these:
* Calculated in some way
* Fixed address (say for some HW IO)
* Most likely from PMM allocation function.

In none of ways listed above is it possible to get an invalid physical page. If we do get one, then
that is a sign of bug and so assertions to check for NULL physical address within PMM is justified.

And the question who should validate? Well no one in particular, the design must of the system
should be such that explicit checks should not be required (beyond a point). And the assertions
should exist only the module which is responsible for this not spread out everywhere.

Assertions should check logic and policies which must always be held.

Now the question is how do we handle these. Is halting the system a suitable reaction on data
corruption, or should we just kill the offending process?

The kernel data is not specific to any process and so a corruption there would effect every process,
and thus we cannot guarantee in what way the system would behave if we let it continue.

Note that a function failing should not be treated as an impossibility. If there is not enough
memory (exception may be the low level allocations) for say a process or a buffer, these should not
halt the system because such scenarios can occur. In this cases however one can panic if there is no
other way to handle it.

However double free/allocation could be a sign of corruption of data/bug within the allocator and 
should be seen a bug and it expected to halt the system.

I do this story is not yet complete and my understanding is also not fully stable, but I think I can
continue with the basis and decide whether to assert on a case by case basis.

------------------------------------------------------------------------------

##  Different ways to handle error/invalid input in Kernel
categories: note, independent, obsolete
_25 June 2023_

Using assert to validate input is a horrible idea. For the current implementation, failing an assert
freezes the machine. For an operating system this is not a good idea to halt the system completely.

Data should have been pre-validated before reaching an assert, making an assert failure an
impossibility. Failing an assert should be seen as a bug.

This presents two problems:
1. Where should be the checks (and not asserts) be?
2. When writing an OS from the bottom, there is sometimes no visibility who should be doing these
   check on inputs. Which is again a case for my confusion.

### Solution: Think in terms of modules who set policies on data

Working on the previous idea, one can think of policies, instead of data. Modules should check for
policies they introduce. Modules which get data from higher level modules, can assert on these as
they must have been checked higher up. Conversely, higher level modules need not know the policies
introduced by lower level modules. They should check the return value to know if an operation has
failed, then they may decide what to do.

This can ensure orthogonality (between higher level modules and lower level policies).

For example:
A) Policy - **0x000 physical address is invalid**
If we think this policy should be enforced by the PMM, then public functions should check physical
address inputs. Modules higher up, which calls PMM routines, need not bother about this policy and
modules lower (in this case bitmap) can assert if it receives an bit index == 0.
If this was not the case and PMM uses assertions, then modules higher up must be checking for this
policy violation. But who? There is no clear taker. Also if this policy changes (say 0x000 address
is now valid) changes will be much more spread out, instead of just the PMM and lower modules (just
assertions in lower modules) changing in case of the former.

### What is meant by modules?

One functional unit. PMM is one module. Bitmap is not a module, its a data structure being used by
PMM.

**TODO**
Failing an assert should not mean calling panic and halting the system. There can be conditions
where a warning can be enough. If an assert failure is not fatal, print a warning.

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
