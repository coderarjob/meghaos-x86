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
