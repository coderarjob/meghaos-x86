# Megha Operating System V2 - x86
------------------------------------------------------------------------------

## Process management
categories: feature, independent

### Process modes

A process is instance of runnable code in the context of an Operating System. It is not necessarily
need to correlate one to one with executable files. A process run portions in "light-weight"
processes called threads.

Both these types of processes can be run in either Ring 3 or Ring 0, the later is called Kernel
mode. The ability to run whole program or portions in either Ring 0 or 3 is important as no
commercial Operating System will allow this easily.

### Threads

#### How threads and non-thread processes differ

1. It runs within the address space of its non-thread parent (one thread can create other threads
   and they all share the same address space of the parent of the first thread). This means no now
   Page Directory is created for threads.
2. Threads do not run whole binaries/programs, they run arbitrary code from its (already loaded)
   parent. In the other case a binary is loaded, mapped and starts execution from a fixed
   location (Programs must be linked properly so that the entry point is at the right/expected
   location in the binary).

#### What remains the same

1. Specific stack for each process. Threads and non-thread ones, whether in Kernel or not has
   separate stacks.
2. Kernel/non-kernel mode determines which ring the process will be run in.

### Kernel mode processes

This depends on the architecture of the CPU and but here are the broad points.
1. Runs in the same privileged mode as the Kernel.
2. Address space (of code, data and stack) separation remains same as general processes. All
   processes, Kernel mode or otherwise runs in its own address space.
3. Non-Kernel mode process can create Kernel mode processes (thread processes or otherwise).

#### How Kernel mode differs from non-Kernel mode processes

1. Kernel process runs in Ring 0. In x86 architecture this means the Segment registers are so setup
   to select Ring 0 rather than Ring 3.
2. On the x86 architecture, non-kernel mode processes has two stacks, one for user mode another
   for Kernel mode. Kernel mode processes on the other hand has one process stack that is used when
   by Kernel as well (With everything in Ring 0, there is no need for a separate stack).
  
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
