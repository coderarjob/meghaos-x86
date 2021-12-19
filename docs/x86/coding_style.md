## Megha Operating System V2 - x86
## Naming Conventions of Variables and Functions
------------------------------------------------------------------------------
_19 December 2021_

### (1) Naming
#### (1.1) Functions

1. Functions names must be lower case, and multi word function names, must be joined by `_`.
2. Module/File name can be prefixed if that makes sense. 
3. All global functions, that run within kernel space, must have `k_` prefix.

   Example: 
   ```
   FindResults *k_util_search_case_insensitive (char *str, char *pattern);
   void         k_mm_alloc_page                (int count);
   ```
2. Functions that must not be called directly should end with `_ndu` (no direct use).
3. Static functions with file scope, should begin with `s_`.

*Restrictions:*
1. Names starting with with `_` or `__` are reserved.
2. limits.h reserves names sufficed with `_MAX`.
3. Names that end with `_t` are reserved for additional type names.

Ref: [https://www.gnu.org/software/libc/manual/html_node/Reserved-Names.html](GCC Reserved Names)

#### (1.2) Variable

1. All variable names must be lower case, and multi word function names, must be joined by `_`.
2. Global kernel space variables, must have `k_` prefixed.
3. Module/File name can be prefixed if that makes sense. 

   Example: 
   ```
   ADDR k_mm_page0_location;
   INT  k_gdt_location;
   ```

#### (1.3) typedef

New types are necessary in the following conditions:
1. Opaque types
2. Portability reasons. When a type need to change depending on some condition like processor etc.

Regardless of the intended use of the custom type, following rules must be always followed:

* All new types that replace standard types, must be written in all UPPERCASE.
* All new types that are aliases must be written in UpperCamelCase. 
* The new type must not have `_t` postfix. Because such names are reserved by the POSIX standard 
  and also possibility the gcc compiler.  Exception is type names, which are part of the C99 
  standard.
* Always typedef enums and structs.
* Names of the new type and the tag (enum and struct) must be the same.
* No cryptic/shortened names.

Example:

```
###if defined(_X86_)
    typdef unsigned      int UINT
###else
    typdef unsigned long int UINT
###end if

typdef struct MatchResult {
    UINT         count;
    MatchResult *next
} MatchResult;

```

#### (1.4) define

1. All constants are written in Upper case.
2. Defines that mimic function, should be named as such.

### (2) Style
#### (2.1) Indentation

Tabs are 4 characters wide. I set my editor to always expand the tabs to 4 characters.

*Rational:*
Indentation helps separate out the different blocks. 4 spaces, provide the ideal separation, in my
opinion. Indentation of 2 or 8 spaces is too little and too much.

1. Put one single space before ( when calling/defining/declaring a function, after
   if/while/do/for/switch.

   Do not however put a space when the previous character is a bracket of some kind.
   Example:

   *Correct*
   ```
   for (int i = 0; i < 10; i++) 
   {
       do_something ();
       do_something_else ();
   }

   int b = ((sizeof (Gdt) / 8) * 7)

   function b () 
   {
       something ();
   }
   ```

   *Wrong*
   ```
   int b = ( ( sizeof (Gdt) / 8 ) * 7 )

   function b() {
       something();
   }
   ```

2 Use one space around (on each side of) most binary and ternary operators, such as any of these:
  `=  +  -  <  >  *  /  %  |  &  ^  <=  >=  ==  !=  ?  :`
3 but no space after unary operators:
  `&  *  +  -  ~  !`
4 no space before the postfix increment & decrement unary operators:
  `++  --`
5 no space after the prefix increment & decrement unary operators:
  `++  --`
6 and no space around the . and -> structure member operators.

#### (2.2) Function declaration

All declarations in a file, should align the return type, function name and the starting bracket (
in columns. Starting from the 2nd argument, every argument are placed in separate line, aligned 
to the one before it.)

Example:
```
void            is_integer    (char c);
unsigned int    to_integer    (char c);
FindResults    *searchpattern (char *str, 
                               char *pattern); 
```
The asterisk stays with the identifier, not the type.

#### (2.3) Function definition

When defining functions, the return type is placed in a separate line and the function name and its
arguments are placed in the following line. Starting from the 2nd argument, every argument are 
placed in separate line, aligned to the one before it.)

**Rational**:

```
static unsigned long
insert (LinkedList ll, int (* ll_function[])(int, int), int count)
{
    ...
}

static unsigned long
insert (LinkedList ll, 
        int (* ll_function[])(int, int), 
        int count)
{
    ...
}
```

As the above example demonstrates, when one of the argument is a function pointer, the
arguments-in-separate-lines styles is much cleaner. To be consistent I keep this style for every
function.

```
static unsigned long
convert (unsigned long long value, IntegerTypes type) 
{
    ...
}
```

**Rational**
Function definitions can easily be separated from the use of the functions. 
`grep -r \^convert` will quickly find the definition and no call to this function or declaration
will be part of this result. Apart from this, I think it looks less cluttered.

#### (2.4) Brackets

Brackets in every case (functions, for, while, if, etc.) must be in a separate line. 

**Rational**
Consider the following two styles with the `if` statement. 
The example, uses a function, but the same effect can be shown
with `if`, `while`, `for`, `do` loops.

```
if (condition1 <> value1 && 
    condition2 <> value2 && 
    condition3 <> value3) {

    statement1;
    statement2;
}

if (condition1 <> value1 && 
    condition2 <> value2 && 
    condition3 <> value3)
{
    statement1;
    statement2;
}
```
In the 1st style, there is no clear distinction, between the function body and its conditions. The
2nd style, with brackets in a separate line, is better right.

```
void 
MyFunction (int parameterOne, 
            int parameterTwo) {
    int localOne,
    int localTwo
}

void 
MyFunction(int parameterOne, 
           int parameterTwo) 
{
    int localOne,
    int localTwo
}
```
With functions, this is less apparent, but the 2nd style is more clear in my thought.

#### (2.5) Return/Error codes

|--------------------|----------------|--------------------------|-------------------------------|
| Function name type | return type    |  Return value on success | Return value on failure/error |
|--------------------|----------------|--------------------------|-------------------------------|
| verb (print/getc)  |  int           |         >=0              |          <0                   |
|--------------------|----------------|--------------------------|-------------------------------|
| verb (print/getc)  |  pointer       |         > 0              |          =0                   |
|--------------------|----------------|--------------------------|-------------------------------|
| verb (print/getc)  |  unsigned int  |                       do not use                         |
|--------------------|----------------|--------------------------|-------------------------------|
| predicate          |  bool          |         true             |         false                 |
|--------------------|----------------|--------------------------|-------------------------------|
| predicate          |  int           |         =0 (false)       |          <0                   |
|                                     |         >0 (true)        |          <0                   |
|--------------------|----------------|--------------------------|-------------------------------|
In each case, errno is set to indicate the appropriate error.

------------------------------------------------------------------------------
_1st November 2020_

** OBSOLETE **
** FOLLOW THE NEW C STYLE GUIDE FROM DEC 19 2021 **

I have been following the [https://github.com/guilt/KOS][KOS], for its 
simplicity. For example routine that prints one character directly on the VGA 
buffer is named `putc`. I think this is too simple for my purpose, as the 
standard `putc` function puts characters into a file descriptor not to VGA 
buffer. We will see this pattern over and over that implementation of kernel 
function and userland functions are different. 

### Functions

* Functions that are meant to be used only in the kernel will have `k` either 
  in prefix or suffix. 
* The function names generally should not tie with implementation.

  1. `kvga_putc()` is bad.
     `kdisp_putc()` is better. As it does not tie the name with VGA. What is CGA
     is used?
  2. `kmalloc()` instead of `malloc()` in kernel implementation.

* Static functions are also named normally with no indication of scope.

* Variables and function with `__` in front are for reserved for special
  purposes. Example: `__kpanic` or `__main` functions.

* Global helper functions (functions that exist to be called from another
  macro) should have `__` in front. For example: `__kpanic()` function is the
  helper function, being called from the `kpanic()` macro.

### Variables

* Global extern variables are named with a prefix `g_`. All external variables
  must be explicitly declared in the file that they are being used, with the
  `extern` keyword.
* Global static variables are named normally.

### Header files

* We do not want the kernel code to include too many header files all the time.
  Common header files should be included into the `kernel.h`. 
* Platform specific header files should exist inside a folder properly named. 
  **Example:** x86 `kernel.h` should be placed inside `include/x86/kernel.h`.

