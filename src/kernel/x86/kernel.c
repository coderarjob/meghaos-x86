/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel 
*
* This is a test kernel to demostrate that the 'boot1' loader can actually load
* the kernel and jump to it.
* On successful execution, 'Kernel ready..' will be displayed on the top of the
* screen.
* ---------------------------------------------------------------------------
*
* Dated: 20th September 2020
*/

#include <kernel.h>
#include <stdarg.h>

static void usermode_main();
void __jump_to_usermode(u32 dataselector, 
                        u32 codeselector, void(*user_func)());
static void div_zero();
static void sys_dummy();
static void fault_gp();
static void page_fault();
static void display_system_info();

/* Only the first 4 MB (0x0000 to 0x3FFFFF) is currently mapped.
 * So accessing 0x400000, causes page fault*/
volatile char *a = (char *)0x400000;

__attribute__((noreturn)) 
void __kernel_main()
{
    kdisp_init();
    printk(PK_ONSCREEN,"\r\n[OK]\tPaging enabled.");

    // TSS setup
    printk(PK_ONSCREEN,"\r\n[  ]\tTSS setup.");
    ktss_init();
    printk(PK_ONSCREEN,"\r[OK]");

    // Usermode code segment
    printk(PK_ONSCREEN,"\r\n[  ]\tUser mode GDT setup.");
    kgdt_edit(GDT_INDEX_UCODE, 0, 0xFFFFF, 0xFA, 0xD);
    // Usermode data segment
    kgdt_edit(GDT_INDEX_UDATA, 0, 0xFFFFF, 0xF2, 0xD);
    kgdt_write();
    printk(PK_ONSCREEN,"\r[OK]");

    // Setup IDT
    printk(PK_ONSCREEN,"\r\n[  ]\tIDT setup");
    kidt_init();
    kidt_edit(0,div_zero,GDT_SELECTOR_KCODE,
              IDT_DES_TYPE_32_INTERRUPT_GATE,0);
    kidt_edit(14,page_fault,GDT_SELECTOR_KCODE, 
              IDT_DES_TYPE_32_INTERRUPT_GATE,0);
    kidt_edit(13,fault_gp,GDT_SELECTOR_KCODE, 
              IDT_DES_TYPE_32_INTERRUPT_GATE,0);
    kidt_edit(0x40,sys_dummy,GDT_SELECTOR_KCODE,
              IDT_DES_TYPE_32_INTERRUPT_GATE,3);

    printk(PK_ONSCREEN,"\r[OK]");

    // Display available memory
    display_system_info();

    // Jump to user mode
    printk(PK_ONSCREEN,"\r\nJumping to User mode..");
    kdisp_ioctl(DISP_SETATTR,disp_attr(BLACK,CYAN,0));
    __jump_to_usermode(GDT_SELECTOR_UDATA, 
                       GDT_SELECTOR_UCODE,
                       &usermode_main);
    while(1);
    
}

void display_system_info()
{
    struct boot_info *mi = (struct boot_info*)BOOT_INFO_LOCATION;

    printk(PK_DEBUG,"\r\nLoaded kernel files:");
    for (int i = 0; i < mi->filecount; i++){
        struct file_des file = mi->files[i];
        printk(PK_DEBUG,"\r\n* file: Start = %x, Length = %x",
                file.startLocation, file.length);
    }

    printk(PK_DEBUG,"\r\nBoot info structure:"); 
    u64 available_memory = 0;
    for(int i = 0; i < mi->count; i++)
    {
        struct mem_des item = mi->items[i];
        available_memory += item.length;
        printk(PK_DEBUG, "\r\n* map: Start = %llx, Length = %llx, Type = %u",
                         item.baseAddr, item.length, item.type);
    }

    printk(PK_ONSCREEN,"\r\nAvailable memory: %u KiB",available_memory/1024);
}

__attribute__((noreturn))
void page_fault()
{
    register int fault_addr;
    int errorcode;
    /* GCC does not preserve ESP in EBP for function with no arguments.
     * This is the reason I am doing ESP+0x24 (listing shows GCC does ESP-0x24
     * as the first instruction in this function)
     * TODO: Implement the page_fault handler in assembly and then call a C
     * function for printing messages etc.
     * */
    __asm__ volatile ("mov %%eax, [%%esp+0x24]\r\n"
                      "mov %0, %%eax":"=m"(errorcode)::"eax");
    __asm__ volatile ("mov %0, %%cr2":"=r"(fault_addr));

    kpanic("Page fault when accessing address 0x%x (error: 0x%x)",
            fault_addr,errorcode);
}

void sys_dummy()
{
    printk(PK_ONSCREEN,"\r\nInside sys_dummy routine..");
    outb(0x80,4);
    // Needs to IRET not RET
}

__attribute__((noreturn))
void fault_gp()
{
    kpanic("%s","General protection fault."); 
}

__attribute__((noreturn))
void div_zero()
{
    kpanic("%s","Error: Division by zero"); 
}

void usermode_main()
{
    //__asm__ volatile ("int 0x40");

    printk(PK_ONSCREEN,"\r\nInside usermode..");
    printk(PK_ONSCREEN,"\r\n%u,%x,%o,%s,%%",
                        45789,
                        0xcafeefe,
                        02760,
                        "Hello Arjob");
    u64 var = 0xCF010203040506FF;   
    printk(PK_ONSCREEN,"\r\n%llx",var);
    printk(PK_ONSCREEN,"\r\nLocation of __kernel_main = %x",__kernel_main);

    //kassert(("Nonsense error",1<0),"Nonsense");
    *a = 0;    

    while(1);
}

