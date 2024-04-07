# Megha Operating System V2 - x86
------------------------------------------------------------------------------

## Operating System ABI
categories: feature, x86

### General

* When Kernel passes control to the new Process, the GP registers can have any value, except ESP and
  EBP. ESP will point to the process stack top and EBP will be zero.
* Stack and Data segment registers must be the same (at all time, this a limitation of  modern
  Compilers)
* Return value is passed using EAX register. Can also be garbage for `void` functions.
* CDECL calling convention is followed.

### System call

#### Passing of arguments
Arguments to system calls are passed via GP registers. The EAX register is used to pass the syscall
function number to the syscall dispatcher routine. Arguments to the syscall routine are passed via 
EBX, ECX, EDX, ESI, EDI registers respectively.

#### Return value from the system call
Apart from the EAX register, syscall routines can also write to a pointer argument to pass values
that way.

#### Preservation of states

The syscall dispatcher routine preserves the state of ESP, EBP, EBX, ECX, EDX, ESI, EDI registers. 
EAX is not preserved and can be garbage if the system call routine returns void. It also does not 
preserve segment, floating point, MMX etc registers.

#### Signature of system call routines

Because of interrupt gate, the processor pushes an Interrupt frame on the stack (kernel stack). This
interrupt frame contains, among other things, the return address.

Pointer to this interrupt frame is always the first argument and only mandatory argument in the
system call signature. After this one there can be at most 5 arguments of U32 type. Not every system
call must have all 5 argument, that is the maximum amount.

So all of these are valid signatures:
```
void sys_foo_1 (InterruptFrame* frame);
void sys_foo_2 (InterruptFrame* frame, U32 type, char* text);
```

------------------------------------------------------------------------------
