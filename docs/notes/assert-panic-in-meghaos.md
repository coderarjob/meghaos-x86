# Megha Operating System V2
----------------------------------------------------------------------------------------------------

## Asserts and Panics in Unittests in MeghaOS
categories: note, independent
_20 July 2023_

In normal condition, when an assert fails, control is transfered to `k_panic`, which prints a panic
message and halts the system. In unittests however the dummy/fake panic routine, prints a message
but returns, which means control will get back and continue from the place where the assert failed.

Thats the problem.

Well one easy solution is to have a global flag which is cleared in the test case implementation
before calling the function under test and will be set in the later on assert failure.

The in the function we check if the global panic flag gets set, then return.


