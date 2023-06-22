## Megha Operating System V2 - x86
---------------------------------------------------------------------------------------------------

## Cross platform guidelines
categories: guideline, independent

To make MeghaOS cross platform, we need to ensure little or no direct use (also called coupling) of
platform dependent (PD) code in platform independent (PI) parts of the OS. The following guidelines
will ensure that at the time of adding new architecture there is little change to be made in PI
codes.

1. PI code must not call/use PD resources/items directly. Although the reverse is accectable and
will cause no issues. If there is indeed a need to directly use a PD resource (in PI code), then
here are some ways to do that:
    a. Introduce an PI abstraction, implemented for each platform in platform dependent way. The
       abstraction itself will be platform independent. See below point 2.
    b. Use `#if` guards to enclose uses of PI items. When adding a new arch,
       new guards need to added in the file. Even though this makes such files no completely be
       PI, this is a simple solution with no extra headers or abstractions. I am one man, I have
       to cut corners sometimes.
2. Types/functions/macros which must be present in every arch but whose implementation are platform
dependent.
    a. We can use PI opaque types or if possible use unions to make the PI type large enough for any
       cast from PD implementation to not fail. Like the implementation of Socket addresses in
       Berkly socket. At this point there are no such example from MeghaOS.
    b. Declare a function in a PI header file, but implement it in a PD C source file.
       This is done for example in the case of `kdisp_ioctl`. It is defined in `include/disp.h`
       whose implementation is in `src/kernel/x86/vgadisp.c`
    c. Declare typedefs/macros in a PD header file and include the header file in another PI header
       file. As the typedefs/macros will be present in every arch, PI part of code can use them
       without any problem.
       Examples are PI header `include/types.h` includes PD header `include/x86/types.h` which have
       concrete implementations for x86 arch. `include/config.h` is another example.
