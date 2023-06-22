## Megha Operating System V2 - x86
------------------------------------------------------------------------------

## Interrupt handling
categories: feature, x86

## Processor Interrupt frame

```
Offset  Register
------  --------------
0       SS                      <-- High memory
4       ESP
8       EFLAGS
12      CS
16      EIP
20      Error code (not always) <-- Low memory
```

## OS Interrupt frame

```
Offset  Register
------  --------------
0       EAX             <-- High memory
4       ECX
8       EDX
12      EBX
16      Original ESP
20      EBP
24      ESI
28      EDI
32      DS
36      ES
40      FS
44      GS              <-- Low memory

Frame Size = 4 * 12 = 48 Bytes
```

The asm handler routine must preserve all the registers before calling the C handler function. This
is because every register are not 'Caller preserved' according to the C ABI.

> System V ABI:
> Registers EBP, EBX, EDI, ESI and ESP 'belong' to the calling function. In other words, a called
> function must preserve these registers’ values for its caller.
> Remaining registers 'belong' to the called function. If a calling function wants to preserve
> such a register value across a function call, it must save the value in its local stack frame.

We keep things simple and preserve every general purpose as well as segment registers. Segment
registers as can be seen from the ABI statement, are also not preserved.

EFLAGS register is not preserved in the OS interrupt frame as it is preserved by the processor in
the Processor's Interrupt frame before calling the interrupt handler.
