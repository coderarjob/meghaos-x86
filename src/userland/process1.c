#include <kdebug.h>
void userland_main()
{
    kbochs_breakpoint();
    while(1);
}
