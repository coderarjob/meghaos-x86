## Megha Operating System V2 - x86
## Where are opaque data types needed?
------------------------------------------------------------------------------
_19 July 2022__

I was thinking of making the GDT and IDT descriptor types opaque. I thought this would prevent
unintentional changes to the fields, as the fields of opaque types cannot be accessed directly.

However, this is not to say that such change cannot be done. Setter methods can be used to change
the fields indirectly, if they exist. Casting a opaque type pointer to any 'defined' type, one can 
change the memory location directly.

So by no means, opaque types can be treated as a 'memory protection' mechanism. The purpose of
opaque types are mostly in user libraries. The abstract opaque type hides the implementation of the
structures, which means they can be changed without effecting the user programs. That is field
can be renamed, added or removed, and user programs will not have to be recompiled or changed in 
any way.

Abstraction can be useful in the kernel as well.

1 As different parts of the kernel interact with each other, using abstractions for these
  interactions will make it easier to manage dependencies in future. Changes to one part of the
  kernel will not directly impact any other part.

2 As fields of the structures cannot be accessed directly, methods must be written for each setting
  and getting of the fields. These methods need to be defined in the same file where the structures
  are defined. This has some advantages and disadvantages.

  - These accessor methods, form the interface for the structure. This interface will hide changes
    to the structure from the rest of the kernel.

  - Another advantage is that these methods can perform some checks before accessing the fields.
    This cannot be done when accessing the fields directly.

Disadvantages of using opaque types in the kernel.
1 One disadvantage which comes to my mind, is that the complexity of creating the accessor
  methods. Accessing the fields directly is 'direct', no supporting methods are required.

2 Fields can only be accessed with these accessor methods. If there is no such method for a
  field, then it cannot be accessed by other part of the kernel.

3 Writing extensive unittests on the structures is a problem as the fields cannot be accessed 
  directly. 

To remedy the disadvantages, the abstract structures are moved to a separate header file, named as 
`<module>_struct.h`. Any C file which includes this header, can access the fields directly and is
no longer abstract type to this file. So in general use, this header file is only included in the 
`<module>.c` and `<module>_test.c` unittest file as both these files require access to the fields.

So from the above points, I think it is obvious that abstraction can be helpful in making the OS
more robust to change and therefore opaque types have a place in kernel development.
