# Megha Operating System V2 - x86
------------------------------------------------------------------------------

## Process management
categories: feature, independent

### New process

* When Kernel passes control to the new Process, the GP registers can have any value, except ESP and
  EBP. ESP will point to the process stack top and EBP will be zero.
  
### Problem with creating kernel modules

At this point, processes can be run in ring 0 and therefore will be able to whatever the kernel can
do. However this does not make it a kernel module. They must provide functionality which user
processes can access. For example drivers can be a Kernel module.

This can be done in two ways:
1. Like many micro kernels:
   The flow of control will look something like this:

   ```
    User process -> (via syscall) Kernel -> process switch -> (via messages) Kernel module -> 
    (via messages/syscall) Kernel -> process switch -> (via syscall) use process.
   ```
   
   It can be implemented relatively easily, but the problem will be slow performance. The two
   process switches per command will cause flushing of caches and slow down everything.

2. Link kernel modules into user process address space:
   The flow of control will look something like this:

   ```
    User process -> (via syscall) Kernel -> Kernel module -> (via syscall) use process.
   ```
  
   Just like the Kernel is mapped into every process, if kernel modules can also be mapped then
   processes can call into routines of kernel modules (via the kernel) with any process switching.

   The problem is implementation. The Kernel can be mapped to each process because there is only one
   kernel and therefore a fixed virtual mapping will work. However as there can be multiple Kernel
   modules, where can they be linked. Note that the VMA at the time of linking determines the paging
   setup, so all kernel modules will be linked to the same VMA, there can only be one Kernel module
   linked to a process.
   
   A dynamic loader is required which can relocate addresses at the time of loading.
