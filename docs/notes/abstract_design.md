## Megha Operating System V2 - x86
## Physical Memory allocation
------------------------------------------------------------------------------
_25 June 2022_

MeghaOS is after all a learning project, this means it should be easier to change parts of the OS
and still be robust.

If I think about it, there already exists one architecture which allows change. It is the device
driver model. What is so special about this model? Abstraction.

So my plan is to have almost every part of the OS be behind an abstraction. Other parts of the OS
will talk to this abstraction when it wants to talk to that part. From Physical Memory manager to
scheduling, should have this abstraction. What will come out of this is that the implementation can
be changed without effecting the rest of the OS (provided the implementation worked well, but there
should be no integration issues).

And if integrated to some devFS, sysFS etc, these modules will be available to the users as files.
This will amaizing be to be able to to read say the PAB (Physical Allocation Bitmap) by reading a
file.

Let me give an example of Physical Memory Manager:

```
    PMM (Implementation)    PWM (Abstraction)     Kernel      User Programs
        ^............................^  ^----------^ ^----------------^
                Implements                        Calls

    ====================            ============================================
    kernel/x86/pmm.c                kernel/x86/interface/pmm.c
    ====================            ============================================
    var        *g_pam               global func get_PAM
    staic func  allocate            global func allocate
    static func deallocate          global func deallocate
    static func init                global func pmm_set_operations
    ====================            global struct {pam, alloc, dealloc} pmm_op
                                    static var *op
                                    ============================================
```

Just like drivers, pmm module defines and fills an instance of pmm_op and calls
pmm_set_operations to set a pointer and from that time, calls to allocate (global) will call
the allocate method in the implementation.

If there is another implementation, it will simply call pmm_set_operations to set it self as
the active implementation, then from that point in time on, it will reveive the allocate and
deallocate calls from the abstraction.

**Note:**
The methods in the abstraction can only be called from programs running the kernel space.
User programs cannot call the abstraction methods directly. They must either interact with the
correcponsing file or call a system call if available.
