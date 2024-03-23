# Megha Operating System V2 - x86
------------------------------------------------------------------------------

## Recursive mapping and temporary mapping in Kernel
categories: note, x86
_24 Feb 2024_

### Introduction

When you add an entry to a PDE (which is a physical page), if recursive mapping is setup you will
find two Virtual addresses for the physical page. One using the PDE you just added and another
because of the recursive mapping setup.

### Notation used

| Notation   | Description                                               | Example                |
|------------|-----------------------------------------------------------|------------------------|
| A[X]       | Access item at index X. Each of the items is of type A.   | PDE[768]               |
| [A:B:C] X  | Virtual address X from PDE[A], PTE[B] and offset C.       | [1023:0:0] 0xFFC0_0000 |
| A[X], B[Y] | Nested access item at index Y (of type B) within the item | PDE[768]PTE[0]         |
|            | at index X (of type A).                                   |                        |

### Basics uses of Recursive mapping

Now the kernel uses the PDE[1023] for recursive mapping. This allows us to do the following:

##### To access each PDE like an array:
```
PDE[0] to PDE[1023] -> [1023:1023:0000] 0xFFFF_F000 to [1023:1023:1023] 0xFFFF_FFFC
```

##### To Access each PTE like an array 
```
PDE[0],   PTE[0] to PTE[1023] -> [1023:0000:0000] 0xFFC0_0000 to  [1023:0000:1023] 0xFFC0_0FFC.
PDE[1],   PTE[0] to PTE[1023] -> [1023:0001:0000] 0xFFC0_1000 to  [1023:0001:1023] 0xFFC0_1FFC.
PDE[768], PTE[0] to PTE[1023] -> [1023:0768:0000] 0xFFF0_0000 to  [1023:0768:1023] 0xFFF0_0FFC.
PDE[769], PTE[0] to PTE[1023] -> [1023:0769:0000] 0xFFF0_1000 to  [1023:0769:1023] 0xFFF0_1FFC.
```

Here is the actual mapping at kenrel initialization (after enabling recursive mapping)

```
0x00000000c0000000-0x00000000c01fffff -> 0x000000000000-0x0000001fffff
0x00000000fff00000-0x00000000fff00fff -> 0x000000044000-0x000000044fff
0x00000000fffff000-0x00000000ffffffff -> 0x000000043000-0x000000043fff
```

### Temporary mapping pages

Temporary mapping is used to map a physical page into the current kernel address space. I say 
'Kernel' address space and not 'current' address space because the Kernel page table is used. This
means the temporary map is not 'per-process'. Previously this was not the case when one entry in the
PD of the current process was used for temporary map. However that approach had the issue where 
seemingly sporadic changes happened to the address space as MMU is interpreting the contents of the
temporarily mapped as a valid page table.

Anyways the current mapping is 
```
PDE[768], PTE[1023] -> [768:1023:0000] 0xC03F_F000 to [768:1023:1023] 0xC03F_FFFC
```

This temporary mapping does not have user access (only kernel can read/write to this page).
