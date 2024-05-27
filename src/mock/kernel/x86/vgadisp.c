#include <mock/kernel/x86/vgadisp.h>
#include <unittest/fake.h>

DEFINE_FUNC_VOID(kdisp_putc, CHAR);
DEFINE_FUNC_VOID(kdisp_scrollDown);
DEFINE_FUNC_VOID(kdisp_init);

/* kdisp_ioctl has a veridac parameter. There is no way to define such a
 * function in fake.h*/

void resetVgaDisp()
{
    RESET_FAKE(kdisp_putc);
    RESET_FAKE(kdisp_scrollDown);
    RESET_FAKE(kdisp_init);
}
