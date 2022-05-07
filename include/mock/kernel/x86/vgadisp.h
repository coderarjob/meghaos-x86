#ifndef VGADISP_FAKE_H
#define VGADISP_FAKE_H

#include <unittest/fake.h>
#include <kernel.h>

DECLARE_FUNC_VOID_1(kdisp_putc, IN, CHAR);
DECLARE_FUNC_VOID_0(kdisp_scrollDown);
DECLARE_FUNC_VOID_0(kdisp_init);

#endif // VGADISP_FAKE_H
