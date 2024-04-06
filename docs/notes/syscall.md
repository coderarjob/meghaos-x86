# Megha Operating System V2 - x86
------------------------------------------------------------------------------

## System call ABI
categories: feature, x86

### Different way to switch privilege levels in x86 processors

* Call gate - Call gates are used to switch between 16 and 32 bit modes but are not used elsewhere.
* Task gate - The Task gates rely on TSS segments, which are used in bare minimum in MOS.
* Interrupt - We use this kind of gates for both HW and SW interrupts. Other interrupts are not
    processed until the current one exits.
* Trap gates - These are not used. But in general they are same as Interrupt gates, just that they
    allow interrupts to occur while inside an interrupt handler.

### Passing of arguments
Arguments to system calls are passed via GP registers. The EAX register is used to pass the syscall
function number to the syscall dispatcher routine. Arguments to the syscall routine are passed via 
EBX, ECX, EDX, ESI, EDI registers respectively.

### Return value from the system call
Return value is passed via the EAX register. Syscall routines can also write to a pointer argument
to pass values that way.

### Preservation of states

The syscall dispatcher routine preserves the state of ESP, EBP, EBX, ECX, EDX, ESI, EDI registers. 
EAX is not preserved and can be garbage if the system call routine returns void. It also does not 
preserve segment, floating point, MMX etc registers.

### Signature of system call routines
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
