## Megha Operating System V2 - x86 - Updated
------------------------------------------------------------------------------

### 9th Nov 2020

* assert.h holds parent assert functions. Assert routines calls `kpanic` if
  some predicate is **false**.
    kstatic_assert(t) and kassert(t,e)

* Other assert functions must ultimately call the `kassert()` function. 
  For example `kassert_perror()` calls `kassert()` when `kerrno != ERR_NONE`.

* All assert functions must respect the DEBUG/NDEBUG macros, and must produce
  no output if DEBUG macro is not defined.

* Every place where must be a hard stop, `kpanic` must be called. Do not use
  `kassert` there.

* There is no `cross platform kernel.c` file, becuase it is not required at
  this point of time. When and if needed in future, it will be created.

* Think of Terry A. Davis. His operating system may had many faults, but it was
  original and upto the idea he had in his mind. He wanted a 64 bit C64 and he
  did exactly that.

  The MeghaOs is the Operating System for my use. And it is okay to make
  mistakes.
