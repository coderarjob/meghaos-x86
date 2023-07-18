# Megha Operating System V2 - x86
------------------------------------------------------------------------------

## Is it required for all the structures to be opaque?
categories: note, independent
_26 March 2023_

Opaque structure are beneficial for the following cases:
1. As a form of abstraction, it can be used to hide implementation details (field names) from the
   users of the structure. If the field names or structures change, the function names need not
   change.
2. Getter and setter methods can do checks and take action. These make bugs easier to catch.

Opaque structure are not beneficial for the following cases:
1. Enforce read-only/private fields. Structures can still be changed by pointer manipulation.
2. Prevent assigning to wrong field. This is programmers logical error and opaque types do little to
   recognise mistakes - Instead of using a wrong field, they now can use wrong getter or setter.
3. Protection against mischievous behaviour.

**Example of how opaque types cannot enforce read-only fields**
```c

/* =========================================================================
 * person.c
 * =========================================================================*/

// Person.id is supposed to be read-only.
struct Person {
    int id;
};

// Initializes a global `thePerson` variable. This simulates some global variable in MeghaOS for
// example BootMemoryMapItem.

void Person_init(int id) {
    thePerson->id = id;
}

int Person_getID(Person* p) {
    return p->id;
}

/* =========================================================================
 * person.h
 * =========================================================================*/

typedef struct Person Person;
extern Person *thePerson;

void Person_init(int id, char *name);
int Person_getID(Person* p);
char* Person_getName(Person* p);

/* =========================================================================
 * main.c
 * =========================================================================*/

// Function which increments memory at a pointer.
void increment(int *v) {
    (*v)++;
}

void main () {
    Person_init(10);                              // global variable `thePerson` is initialised.
    printf("ID: %d\n", Person_getID(thePerson));  // prints ID: 10

    increment((int*)thePerson);                   // Wrongfully passing thePerson to increment.
                                                  // Instead of an error thePerson.id was changed.
    printf("ID: %d\n", Person_getID(thePerson));  // prints ID: 11
}
```

Lets examine which types in Megha need to be `Opaque`.

1. BootMemoryMapItem

* Structure depends on: BIOS call. Prime target for abstraction this will change for different
  processor architectures and BIOS.
* Is used in architecture independent parts of the Kernel.

**Need to be Opaque: Yes**

2. BootFileItem, BootLoaderInfo
* Structure depends on: BOOT1. Could be made architectures independent or at least a standard for
  other architectures as information contained in these structures do not directly dependent on the
  system.
* However, this structure cannot be passed by value, as it will always contains a pointer to an
  abstract type. 
* Globally declared. Which means everyone have access to it always and prone to be changed by
  mistake.

**Need to be Opaque: Yes**

3. IDT, GDT
* Structure depends on: Processor Arch.
* Is only used by processor dependent parts of Kernel.
* The GDT or IDT pointers are locally declared and cannot be accessed by anyone.
* If required structure can be passed by value.

**Need to be Opaque: No**

4. PageTableEntry, PageDirectoryEntry4MB, PageDirectoryEntry4KB
* Structure depends on: Processor Arch.
* Is used in architecture independent parts of the Kernel.

**Need to be Opaque: Yes**

At this point however, thinking too much about opaque types is premature. Currently the OS is in its
very early days and I do not have clarity to take decisions on these things. Lets just keep the OS
as it is and not remove or introduce opaque types until a time when I can think of it.

------------------------------------------------------------------------------

## Why are opaque data types needed?
categories: note, independent
_19 July 2022_

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


