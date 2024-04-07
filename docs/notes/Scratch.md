# MEGHA OPERATING SYSTEM V2 - x86
----------------------------------------------------------------------------------------------------

## SCRATCH PAD FOR QUICK NOTES

#### Notation used

| Notation   | Description                                               | Example                |
|------------|-----------------------------------------------------------|------------------------|
| A[X]       | Access item at index X. Each of the items is of type A.   | PDE[768]               |
| [A:B:C] X  | Virtual address X from PDE[A], PTE[B] and offset C.       | [1023:0:0] 0xFFC0_0000 |
| A[X], B[Y] | Nested access item at index Y (of type B) within the item | PDE[768]PTE[0]         |
|            | at index X (of type A).                                   |                        |

------------------------------------

## System call stack in User and Kernel processes
categories: note, obsolete, x86
_097 April 2024_

NEED TO BE REVISED

### System call - User process

| Register state                         | Where                     |
|----------------------------------------|---------------------------|
| esp: 30fef                             | INT 0x50                  |
| ebp: 30ffb                             |                           |
| Ring 0 stack switched. (Base: 0x22FFF) | At syscall_asm_despatcher |
| 20 bytes of interrupt frame pushed     |                           |
| esp: c0022feb                          |                           |
|----------------------------------------|---------------------------|

* esp: 30fef
* ebp: 30ffb
* INT 0x40
* Ring 0 stack switched. (Base: 0x22FFF)
* 20 bytes of interrupt frame pushed
* esp: c0022feb
* One push eab
* 48 bytes of OS interrupt frame pushed
* esp: c0022fb7
* Push interrupt frame address (argument)
* call sys_dummy_handler
* One push eab
* esp: c0022fab

At the end of the interrupt routine control will reach the instruction after INT 0x40 and at that
time ESP, EBP should be what it was at the start 0x30fef, 0x30ffb.

System call - Kernel process

* esp: c0022f87
* ebp: c0022f93
* INT 0x40
* 12 bytes of interrupt frame pushed
* esp: c0022f7b
* One push eab
* 48 bytes of OS interrupt frame pushed
* esp: c0022fb7
* Push interrupt frame address (argument)
* call sys_dummy_handler
* One push eab
* esp: c0022f3b

At the end of the interrupt routine control will reach the instruction after INT 0x40 and at that
time ESP, EBP should be what it was at the start 0xc0022f87, 0xc0022f93.

------------------------------------

## Different way to switch privilege levels in x86 processors
categories: note, x86
_8 April 2024_

* Call gate - Call gates are used to switch between 16 and 32 bit modes but are not used elsewhere.
* Task gate - The Task gates rely on TSS segments, which are used in bare minimum in MOS.
* Interrupt - We use this kind of gates for both HW and SW interrupts. Other interrupts are not
    processed until the current one exits.
* Trap gates - These are not used. But in general they are same as Interrupt gates, just that they
    allow interrupts to occur while inside an interrupt handler.

------------------------------------

## Strange behaviour because of Stale TLB
categories: note, x86
_3 March 2024_

Cache related errors are strange and sometimes are difficult to debug. Following describes one such
case, though this one was not too difficult to debug. It seemed to be strange to me at first but
then, with a little digging it became apparent what was going on.

##### Setup

* Temporary mapping is done using [768:1023:0] 0xC03FF000.
* PD is at 0x43000 paddress and PDE[768],PTE[0] is 0x44000 paddress.
* TLB invalidation is disabled in paging.c throughout.

##### Observation

1. We get a page aligned physical address. Say that is 0x47000.
2. Map this physical address to the temporary address. That is call `kpg_temporaryMap(0x47000)`.

At this point the memory map looks like the following.

```
0x00000000c0000000-0x00000000c01fffff -> 0x000000000000-0x0000001fffff
0x00000000c0200000-0x00000000c0200fff -> 0x000000046000-0x000000046fff
0x00000000c03ff000-0x00000000c03fffff -> 0x000000047000-0x000000047fff
0x00000000fff00000-0x00000000fff00fff -> 0x000000044000-0x000000044fff
0x00000000fffff000-0x00000000ffffffff -> 0x000000043000-0x000000043fff
```

3. Write something to this address. Say for example `0xF001`

Now at this point if you take a look at the memory map, you see something strange. The correct
behaviour would have been the memory map staying the same as before.

```
0x00000000c0000000-0x00000000c0000fff -> 0x00000000f000-0x00000000ffff
0x00000000c0001000-0x00000000c01fffff -> 0x000000001000-0x0000001fffff
0x00000000c0200000-0x00000000c0200fff -> 0x000000046000-0x000000046fff
0x00000000c03ff000-0x00000000c03fffff -> 0x000000047000-0x000000047fff
0x00000000fff00000-0x00000000fff00fff -> 0x000000044000-0x000000044fff
0x00000000fffff000-0x00000000ffffffff -> 0x000000043000-0x000000043fff
```

We see there there is one more row, this extra one is coming because the 0xC0000000 mapping got
splitted. Where previously 0xC0000000 vaddress mapped to 0x000000000 paddress, it now mapped to
0xF000 paddress.

##### Reason and conclusion

This error which is only seen when all TLB invalidation was disabled, is a TLB/Caching related
error.

For vaddress mapping to change its corresponding PTE must change. 0xC0000000 vaddress, corresponds
to PDE[768],PTE[0] (at 0x44000 paddress) entry. When we wrote to 0xC03FF000 vaddress, 0x44000
paddress got written to instead of 0x47000, which we temporary mapped.

This happened because TLB was not flushed before doing the `kpg_temporaryMap` call and this resulted
in MMU using an old map from TLB. It so happens that there was an older call to temporary map 
0x44000 paddress. Even though we have called `kpg_temporaryUnmap` the TLB was not flushed, and this
this issue.

Note that only when we have a PTE with Present = 1, so we have to flush the TLB. Entires where
Present = 0 need not require flushing.
