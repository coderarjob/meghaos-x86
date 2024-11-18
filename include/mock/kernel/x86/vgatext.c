#ifndef VGADISP_FAKE_H
#define VGADISP_FAKE_H

#include <unittest/fake.h>
#include <types.h>

DECLARE_FUNC_VOID(kdisp_putc, CHAR);
DECLARE_FUNC_VOID(kdisp_scrollDown);
DECLARE_FUNC_VOID(kdisp_init);

void resetVgaDisp();
#endif // VGADISP_FAKE_H
