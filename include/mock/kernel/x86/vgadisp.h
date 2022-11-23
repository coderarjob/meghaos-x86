#ifndef VGADISP_FAKE_H
#define VGADISP_FAKE_H

#include <unittest/fake.h>
#include <kernel.h>

DECLARE_FUNC_VOID(kdisp_putc, CHAR);
DECLARE_FUNC_VOID(kdisp_scrollDown);
DECLARE_FUNC_VOID(kdisp_init);
DECLARE_FUNC(bool, kdisp_isInitialized);

void resetVgaDisp();
#endif // VGADISP_FAKE_H
