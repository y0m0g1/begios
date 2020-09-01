#include "bootpack.h"

void init_pic(void)
{
    io_out8(PIC0_IMR,  0xff);   // deny all interruption
    io_out8(PIC1_IMR,  0xff);   // deny all interruption

    io_out8(PIC0_ICW1, 0x11);   // edge triger mode
    io_out8(PIC0_ICW2, 0x20);   // IRQ0-7 <-> INT20-27
    io_out8(PIC0_ICW3, 1<<2);   // connect PIC1 over IRQ2 port
    io_out8(PIC0_ICW4, 0x01);   // none-buffer mode

    io_out8(PIC1_ICW1, 0x11);   // edge triger mode
    io_out8(PIC1_ICW2, 0x28);   // IRQ8-15<->INT28-2f
    io_out8(PIC1_ICW3,    2);   // connect PIC1 over IRQ2 port
    io_out8(PIC1_ICW4, 0x01);   // none-buffer mode

    io_out8(PIC0_IMR,  0xfb);   // only accept the interruption from PIC1 : 11111011
    io_out8(PIC1_IMR,  0xff);   // deny all interruption

    return;
}

void inthandler27(int *esp)
{
    io_out8(PIC0_OCW2, 0x67);
    return;
}