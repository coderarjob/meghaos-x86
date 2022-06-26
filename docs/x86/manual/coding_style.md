## Megha Operating System V2 - x86
## Naming Conventions of Variables and Functions
------------------------------------------------------------------------------
_19 December 2021_

### (1) Naming
#### (1.1) Functions

1. Function verb must be lower Camel case. Do not separate out words with `_`.
2. All global functions, that run within kernel space, must have `k` prefix.
3. After the `k` prefix, module/file name can be placed, in lower case.
4. If module name is written after, 'k', then after the module name, a `_` separates the actual
   function name verb.

   Example: 
   ```
   FindResults *
   kutil_searchCaseIsensitive (char *str, char *pattern);
   
   void         
   kmm_allocPage (int count);
   ```
5. Functions that must not be called directly should end with `_ndu` (no direct use).
6. Static functions with file scope, should begin with `s_`.

**Rational**
The `k` and the `module` name is a quick way to know the that the function influences the kernel
space and the module name prevents weird function name.

```
void
kgdt_add(...) 
{
    ...
}

void
kidt_add(...)
{
    ...
}
```

*Restrictions:*
1. Names starting with with `_` or `__` are reserved.
2. limits.h reserves names sufficed with `_MAX`.
3. Names that end with `_t` are reserved for additional type names.

Ref: [https://www.gnu.org/software/libc/manual/html_node/Reserved-Names.html](GCC Reserved Names)

#### (1.2) Variable

1. All variables must be lower Camel case. Do not separate out words with `_`.
2. Global kernel variables, must have `k` prefix.
3. After the `k` prefix, module/file name can be placed, in lower case.
4. After the `k` prefix and the module name, a `_` separates the actual variable name.
5. Static global variables with file scope, should begin with `s_`.

   Example: 
   ```
   ADDR kmm_page0Location;
   INT  kgdt_location;
   ```
*Restrictions:*
1. Names starting with with `_` or `__` are reserved.
2. limits.h reserves names sufficed with `_MAX`.
3. Names that end with `_t` are reserved for additional type names.

#### (1.3) typedef

New types are necessary in the following conditions:
1. Opaque types
2. Portability reasons. When a type need to change depending on some condition like processor etc.

Regardless of the intended use of the custom type, following rules must be always followed:

1 All new types that replace standard types, must be written in all *UPPERCASE*.
2 All new types that are aliases must be written in *UpperCamelCase*.
4 Always typedef enums and structs.
5 Names of the new type and the tag (enum and struct) must be the same.
6 No cryptic/shortened names.

*Restrictions:*
1. Names starting with with `_` or `__` are reserved.
2. limits.h reserves names sufficed with `_MAX`.
3. Names that end with `_t` are reserved for additional type names.

*Things to consider:*
1. Do not abbreviate the work `kernel` with `k`. Say that need to be said.
2. Write module/namespace names in full or short form that is understood by all.
   Instead of `disp_VgaColors`, write `DisplayVgaColors`
3. Do not use `_` to separate words.

Example:

```
// These are a replacement for the C99 types, so the new type names are all UpperCase.
#if defined(_X86_)
    typdef unsigned      int UINT
#else
    typdef unsigned long int UINT
#end if

// Instead of k_ErrorCodes.
typedef enum KernelErrorCodes
{
    ERR_NONE,
    ERR_BIOS_FAULT,
    ERR_COUNT
} KernelErrorCodes;

// Module name written before the type name nown.
typedef struct UtilityMatchResult 
{
    UINT         count;
    MatchResult *next
} UtilityMatchResult;

```

#### (1.4) define

1. All constants are written in Upper case.
2. limits.h reserves names sufficed with `_MAX`.
3. Names starting with with `_` or `__` are reserved.
4. Defines that mimic function, should be named as such.

### (2) Style
#### (2.0) Limit on length of lines

1. Maximum lenght of lines in documnet files (.md, .txt) is 100 columns.
2. Maximum lenght of lines in assembly files is 80 columns.
3. Maximum lenght of lines in C files is 100 columns.

The Rational for 100 columns limit for C files instead of 80 is to allow for more descriptive
function names and to fit more parameters in one line.

#### (2.1) Indentation

Tabs are 4 characters wide. I set my editor to always expand the tabs to 4 characters.

*Rational:*
Indentation helps separate out the different blocks. 4 spaces, provide the ideal separation, in my
opinion. Indentation of 2 or 8 spaces is too little and too much.

1. Put one single space before ( when calling/defining/declaring a function, after
   if/while/do/for/switch. Exception is macro definition, as the `(` must follow the macro name
   without any spaces.

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

#### (2.2) Function definition

1. When defining functions, the return type is placed in a separate line and the function name and 
   its parameters are placed in the following line. If a pointer is returned, then the asterisk 
   stays with the type, not the identifier (opposite to the way we write the parameters).

    static unsigned long
    convert (unsigned long long value, 
             IntegerTypes type) 
    {
        ...
    }

**Rational**
Function definitions can easily be separated from the use of the functions. 
`grep -r \^convert` will quickly find the definition only, not the declaration or any calls to 
this function. Apart from this, I think it looks less cluttered.

2. The parameter placement in the function definition, can be of two types. Choose the right one as
   per the situation. Both are valid, and will come down to choice.

   A. Each of the parameters are placed in individual lines, all aligned to the previous one. 
   B. Parameters that are alike or part of a group, are placed on the same line. 


**Rational**:

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

As the above example demonstrates, when one of the argument is a function pointer, the
parameters-in-separate-lines styles is much cleaner. 

However, this is not practical when the number of arguments is small or makes sense to keep
together.

    static void 
    setColor(UINT r,
             UINT g,
             UINT b)
    {
        ...
    }

    static void 
    setColor(UINT r, UINT g, UINT b)
    {
        ...
    }

In this case, I think the below style makes more sense. 

In the end the parameter placement comes down to judgement and is a matter of preference.

#### (2.3) Function declaration

All declarations in a file, should align the return type, function name and the starting bracket (
in columns. The placement of the parameter match the parameter placement of the function
definition.

Example:
    void                 is_integer    (char c);
    unsigned int         to_integer    (char c);
    static void          setColor      (UINT r, UINT g, UINT b)
    static unsigned long insert        (LinkedList ll, 
                                        int (* ll_function[])(int, int), 
                                        int count)

The asterisk stays with the identifier, not the type.

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

| Function name type | return type    |  Return value on success | Return value on failure/error |
|--------------------|----------------|--------------------------|-------------------------------|
| verb (print/getc)  |  int           |         >=0              |          <0                   |
| verb (print/getc)  |  pointer       |         > 0              |          =0                   |
| verb (print/getc)  |  unsigned int  |                       do not use                         |
| predicate          |  bool          |         true             |         false                 |
| predicate          |  int           |         =0 (false)       |          <0                   |
| predicate          |  int           |         >0 (true)        |          <0                   |

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

