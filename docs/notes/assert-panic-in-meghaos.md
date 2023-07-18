# Megha Operating System V2
----------------------------------------------------------------------------------------------------

## Fixed - Assert/Panic handling in unittests
categories: note, independent
_02 July 2023_

Related to commit: _8f9990948f70_

The below note sets the context for this fix.

Previously, when built for unittests asserts/panics (in the unit under test) would continue
executing and required explicit checks in the interface to return from if `panic_invoked` is true.
    
This was a very bad design because interfaces needed to be aware about unittest setup and must
implement code for unittests to work properly.

The new solution eliminates this drawback. Calling panic macro now returns from the called function.
This 'return' code is written in assembly which unwinds the ESP and calls RET after restoring the
EBP register.

This however is not without its drawbacks:
1. Host specific implementation - Because of the use of assembly instructions, this code need to be
   reimplemented for architectures where unittests is to run.
   NOTE: Its host specific (build system) not target (target CPU of MeghaOS) specific.
2. Only works if CDECL calling convention is used.
3. When panic is called, the function simply returns without settings EAX register. It can be
   garbage.

------------------------------------------

## Asserts and Panics in Unittests in MeghaOS
categories: note, independent
_20 June 2023_

In normal condition, when an assert fails, control is transfered to `k_panic`, which prints a panic
message and halts the system. In unittests however the dummy/fake panic routine, prints a message
but returns, which means control gets back to the unittest routine.

This is a problem.

There are two important questions:

1. Do we need to test assert conditions?
2. If we do need to test asserts, how do we solve the above mentioned problem?

### Do we need to test assert conditions?
I do not have an absolute answer. I just know that some of the asserts validates function arguments.
This is required so that we can detect and fail on any input which does not match the expectation.
So as these asserts validates input, we must test them with invalid inputs. Such tests increases
confidence on the interface as it is getting tested extensively.

## How asserts work in unittests? (Obsolete)
Well one easy solution when building unittests have a global flag which is set within the fake panic
routine and unittest routine tests this flag to check if assert was hit in the interface under test.

The in the interface we check if the global panic flag gets set, then return.

Issue with this approach is the source code is slightly different when building for unittest. This
is not ideal, as unittest should test the code that runs normally, there should be nothing specific
to unittests in the interface we are testing. In our case there is an extra branch in `k_assert` and
extra `ifdef` within the interface.

