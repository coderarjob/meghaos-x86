# Megha Operating System V2 - x86
------------------------------------------------------------------------------

## Process management
categories: feature, independent

### Process modes

A process is instance of runnable code in the context of an Operating System. They does not
necessarily correlate one to one with executable files. A process can also create "light-weight"
processes called threads that run independently by share code & data with the parent process. These
are the two types of processes - 'Threads' & 'Non-thread' processes aka just 'Process'.

Both these types of processes can be run in either Ring 3 or Ring 0, the later is called Kernel
mode. The ability to run whole program or portions in either Ring 0 or 3 is important as no
commercial Operating System will allow this easily.

```
|                                    | Process  | Threads        | Kernel process |
|------------------------------------|----------|----------------|----------------|
| Physical memory (& Page Directory) | Separate | Same as Parent | Separate       |
| Code run level                     | Ring 3   | Ring 3         | Ring 0         |
| Data run level                     | Ring 3   | Ring 3         | Ring 0         |
|------------------------------------|----------|----------------|----------------|
| Text virtual address space (VAS)   | Separate | Same as parent | Separeate      |
| Text VAS privilege level           | Ring 3   | Ring 3         | Ring 3         |
|------------------------------------|----------|----------------|----------------|
| Data Heap VAS                      | Separate | Same as parent | Separeate      |
| Data Heap VAS privilege level      | Ring 3   | Ring 3         | Ring 3         |
|------------------------------------|----------|----------------|----------------|
| Stack VAS                          | Separate | Separate       | Separeate      |
| Stack VAS privilege level          | Ring 3   | Ring 3         | Ring 0         |
|------------------------------------|----------|----------------|----------------|
```

The 'Run Level' is the privilege levels for code & data in a process while its running. If it tries
to access memory for which it does not have permission, CPU will through an exception. Ring 0 is the
most privileged, so a process running in Ring 3 cannot have execute Ring 0 specific instructions &
memory pages. Megha OS however allows any process/thread to run in the most privileged, Ring 0 mode
allowing it to access hardware and memory locations for playing around and experimenting.

The 'Virtual page privilege levels' is the privilege level of virtual memory pages. Memory used by
the hardware or the Kernel are always Ring 0, which means a process running in Ring 3 would not be
able to access them.

Note that the within a process's virtual address space, the pages for Code & Data are always Ring 3,
this is so that a Non-Kernel thread can run code & access data of its Kernel process.

### Process creation

Every process has three regions of memory, the Text, Data Heap and Stack. The placement in VAS and
their size is shown in the table below.

```
|                     | Process                              | Threads                   |
|---------------------|--------------------------------------|---------------------------|
| Text VAS start      | ARCH_MEM_START_PROCESS_TEXT          | Function address          |
| Text VAS size       | Provided                             | n/a  (Text is shared)     |
|---------------------|--------------------------------------|---------------------------|
| Data Heap VAS start | Dynamic                              | n/a (Data heap is shared) |
| Data Heap VAS size  | ARCH_MEM_LEN_BYTES_PROCESS_DATA      | n/a (Data heap is shared) |
|---------------------|--------------------------------------|---------------------------|
| Stack VAS start     | Dynamic                              | Dynamic                   |
| Stack VAS size      | ARCH_MEM_LEN_BYTES_PROCESS_STACK + 2 | :- Same as Process :-     |
|---------------------|--------------------------------------|---------------------------|
```

The VAS for each process starts from `ARCH_MEM_START_PROCESS_MEMORY` (4 KB) and ends at
`ARCH_MEM_END_PROCESS_MEMORY` (3GB). The Kernel VAS is from 3GB to 4 GB.

As the table above shows Threads run the same memory context as its parent, it shares the Physical
memory pages and Virtual Memory addresses. Threads do however have separate Stacks.

Processes (non threads) programs must be linked properly so that the entry point is at the expected
location in the binary. Memory for Stack & Data heap gets allocated dynamically but always have a
fixed maximum size.

The memory for stack has guard pages at the top & bottom to catch stack overflow & underflow.

Kernel/non-kernel mode determines which ring the process will be run in. A process running in
non-Kernel mode process can create Kernel mode processes (thread processes or otherwise).

Kernel processes runs in Ring 0. In x86 this means the Segment registers are so setup to select Ring
0 rather than Ring 3.

### Process Hierarchy

Except for the 'Root' process, every other processes and all threads have a parent. Root process is
the process started by the Kernel. It does not have any parent process and its called root because
all application level processes are started by it or its child processes.

When a non-thread process starts another process (thread or otherwise), it becomes the parent
process of the new process. When a thread process starts another process, it however does not become
the parent, but the parent process of the thread becomes the parent of the new process. This means
that threads cannot have child processes. This simplifies thread exit process as we do not have to
kill/adopt their child processes. Note that threads can still create other process & threads they
just don't own them.

Another restriction on threads is that they can never be the 'Root' process. Threads must always
have a parent. This restriction exits just to keep the process hierarchy always have one single
root.

Furthermore to have a single root process, there is another restriction. 'Root' process can only be
created when there are no current process.

```
| Parent  | Child       | Allowed?                   | Parent                    |
|---------|-------------|----------------------------|---------------------------|
| Kernel  | 1st Process | Yes (Becomes root process) | NULL                      |
| Kernel  | 2nd Process | No (Single root process)   | -                         |
| Kernel  | Thread      | No (must have a parent)    | -                         |
| Process | Process     | Yes                        | Current process           |
| Process | Thread      | Yes                        | Current process           |
| Thread  | Process     | Yes                        | Parent of current process |
| Thread  | Thread      | Yes                        | Parent of current process |
|---------|-------------|----------------------------|---------------------------|
```

### Process Scheduler

`TDB`

### Process Events

Every process has a queue for process events. These events tell the process about some event or
state change. For instance when timer wants the current process to yield it adds
`KERNEL_EVENT_PROCCESS_YIELD_REQ` even to the events queue of the current process.

Each event item contains two fields: Event ID and Event Data.

Every process has its own process & thread have individual event queue.

### Process exit

Exiting threads are the simplest, since they only have a stack, exiting threads means to only
deallocate the virtual & physical memory for its stack, and freeing the memory used by process's
events and removing the thread from the scheduler queue. That is steps 1 to 4 don't happen for
threads but 5,6 does happen.

Exiting process is little bit extended. Its done in the following flow.

1. Recursively exit all its child processes (threads & otherwise).
2. If current process is being killed, then we first switch to use the Kernel Page Directory so that
   the current one can be freed. Its possible to change to Kernel PD seamlessly because Kernel VAS
   is mapped to every process.
3. Delete entire virtual address space of the process. This will also free the physical memory pages
   that were mapped.
4. Deallocate physical memory used for the process's Page Directory and Tables are deallocated.
5. Memory used up by process events are freed.
6. Process is removed from the scheduler queue.

The steps are same for both kernel and non-kernel processes just that no stack switch occurs when a
kernel process makes a system call. This however causes a problem when a killing a Kernel process.
Specifically after virtual memory for its stack gets unmapped it becomes impossible for the function
to return back to complete the rest of the exit process. To kill a Kernel process (thread or
otherwise) there is one additional step that happens before the start of the exit process, that is
before step 1. Since the problem happens only using a different stack would be sufficient, here we
switch to use the Kernel Stack (which is separate from the process's stack). This again works
because the page mapping for the Kernel Stack is already present in every process.

When process exits it passes along a 'exit code', corresponding to the its state. These exit codes
are 8 bit numbers (so it can at most be 255), which are sent to the parent of the process being
killed by pushing a `KERNEL_EVENT_PROCCESS_CHILD_KILLED` event.

Through this event the parent can determine if a child process has exited and in what condition.

When a process exits, it must also kill its child threads. Child non-thread processes however are
not killed and become children of the Root process.

### Abort

Abort is called when there is a serious error or corruption in the application program. For example
malloc() calls abort when it detects a double free/allocation. The Abort call should kill all
processes what is using the current memory context, that is threads and the parent process. Child
non-thread processes however are not killed since they have a different memory context. These
processes get owned by the Root process.

------------------------------------------------------------------------------

## Problem with creation of new process from a kernel process
categories: note, x86, obsolete
10th Nov 2024

When a Kernel process makes a syscall, no stack switch occurs. This causes stack corruption when
trying to switch to the new process.

When we switch to the address space of the new process (when switching to a non-thread process) or
modify the current address space to add a new stack (when switching to a thread process) in the
middle of the `kprocess_switch` function, the state of the current stack is destroyed and inevitably
causes a fault soon.

The solutions I see are these:
1. The `syscall_asm_despatcher` function always switches to the Kernel stack before continuing. This
   solves both the above two problems as the Kernel address space is always available (even in a new
   process) and Kernel stack space is also separate, thus will not be overridden.
   Problem with this approach is that we will not be able to issue a syscall from within the Kernel.
   It would cause stack corruption. However not sure why would we issue a syscall when we are
   already in the Kernel space.
2. Only change the current address space just before jumping to the process. All operations like
   address space setup etc should be done using temporary mapping in the current address space.
   Do not see any problem with this approach, just that I have to think of a way to make these
   temporary mappings, as the existing temporary map function cannot be used for these.

------------------------------------------------------------------------------

## Problem with creating kernel modules
categories: note, x86
10th Nov 2024

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
