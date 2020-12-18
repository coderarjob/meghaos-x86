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

    /* typedefs produce a more consistent result, for complicated types,
     * than #define 
     * Example:
     *    #define p_t char*
     *    p_t a,b,c             // a is char *, b,c are char.
     *
     *   typedef char* p_t;
     *   p_t a,b,c              // a,b,c are char* as it should be.
     * */
    typedef unsigned char u8;
    typedef unsigned short u16;
    typedef unsigned int u32;
    typedef unsigned long long u64;

    typedef u32 size_t;     // Type that can hold the largest number in an 
                            // architecture.
#endif // __x86_TYPES__
