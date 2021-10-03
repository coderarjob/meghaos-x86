/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Platform specific Types 
*
* Note:
* Remember that these header files are for building OS and its utilitites, it
* is not a SDK.
* ---------------------------------------------------------------------------
*
* Dated: 1st November 2020
*/

#ifndef __x86_TYPES__
#define __x86_TYPES__

    /*
      typedefs produce a more consistent result, for complicated types,
      than #define.
      Example:
         #define p_t char*
         p_t a,b,c             // a is char *, b,c are char.

        typedef char* p_t;
        p_t a,b,c              // a,b,c are char* as it should be.
     */
    typedef unsigned char u8;
    typedef unsigned short u16;
    typedef unsigned int u32;
    typedef unsigned long long u64;
    typedef u32 size_t;     // Type that can hold the largest number in an 
                            // architecture.

    // Casts a bit field of n bits to 32 bit unsigned int.
    inline u32 CAST_BITN_TO_U32(u32 t, u32 n) {
        n--;
        return (u32)(t & ((2 << n) -1));
    }

    /*
       VIRTUAL_ADDRESS:
       32 bit Physical address. This type will prevent mixing physical and
       virtual addresses as this type cannot be cast to any other pointer, not
       even void * explicitly.

       Use the 'PHYSICAL_ADDRESS' macro for initializing variables or function
       parameters of type PHYSICAL_ADDRESS.

       Example:
       PHYSICAL_ADDRESS vga = PHYSICAL_ADDRESS(0xB8000);
       Read it as, the 'vga' variable of type PHYSICAL_ADDRESS holds the
       physical address 0xB8000. So think of this type as a pointer
       implementation.
    */

    typedef
    struct { size_t phy_addr; } __attribute__((packed)) PHYSICAL_ADDRESS;

    #define PHYSICAL_ADDRESS(address) {.phy_addr = address}


#endif // __x86_TYPES__
