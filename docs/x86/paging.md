## Megha Operating System V2 - x86
## Paging
------------------------------------------------------------------------------
_18th Dec 2020_

Where should we initiate and start using paging? 
 * In `boot1` or 
 * In `kernel`

I think it should be in `kernel`. This is because of the following reasons.

 1. If at some point I started using GRUB, as the bootloader, the Paging must
    be started from kernel, as GRUB itself does not turn on paging.

 2. If impemented in `boot1`, then the part of the code that comes after, need
    to be properly mapped. 

    **May be `ORG VM_START` is the way to do it**

  3. The `boot1` becomes tied to the details of paging implementation (at-least
     only to be overriden by the `kernel`).
