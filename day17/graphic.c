#include "bootpack.h"

void init_palette(void)
{
    static unsigned char table_rgb[16 * 3] = {
        0x00, 0x00, 0x00, // 0:black
        0xff, 0x00, 0x00, // 1:bright red
        0x00, 0xff, 0x00, // 2:bright green
        0xff, 0xff, 0x00, // 3:bright yellow
        0x00, 0x00, 0xff, // 4:bright blue
        0xff, 0x00, 0xff, // 5:bright purple
        0x00, 0xff, 0xff, // 6:bright light blue
        0xff, 0xff, 0xff, // 7:white
        0xc6, 0xc6, 0xc6, // 8:gray
        0x84, 0x00, 0x00, // 9:dark red
        0x00, 0x84, 0x00, //10:dark green
        0x84, 0x84, 0x00, //11:dark yellow
        0x00, 0x00, 0x84, //12:dark blue
        0x84, 0x00, 0x84, //13:dark purple
        0x00, 0x84, 0x84, //14:dark light blue
        0x84, 0x84, 0x84  //15:dark gray
    };
    set_palette(0, 15, table_rgb);
    return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
    int i, eflags;
    eflags = io_load_eflags();  // record the value of interrupt enable flag
    io_cli();                   // clear(set 0 to) IE flag to disable interruption
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++)
    {
        io_out8(0x03c9, rgb[0]/4);
        io_out8(0x03c9, rgb[1]/4);
        io_out8(0x03c9, rgb[2]/4);
        rgb += 3;
    }
    io_store_eflags(eflags);    // restore the value of IE flag
    return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
    int x, y;
    for (y = y0; y <= y1; y++)
    {
        for (x = x0; x <= x1; x++)
        {
            vram[y * xsize + x] = c;
        }
        
    }
    return;
}

void init_screen8(char *vram, int x, int y)
{
    boxfill8(vram, x, COL8_008484, 0, 0, x-1, y-29);
    boxfill8(vram, x, COL8_C6C6C6, 0, y-28, x-1, y-28);
    boxfill8(vram, x, COL8_FFFFFF, 0, y-27, x-1, y-27);
    boxfill8(vram, x, COL8_C6C6C6, 0, y-26, x-1, y-1);

    boxfill8(vram, x, COL8_FFFFFF, 3, y-24, 59, y-24);
    boxfill8(vram, x, COL8_FFFFFF, 2, y-24, 2, y-4);
    boxfill8(vram, x, COL8_848484, 3, y-4, 59, y-4);
    boxfill8(vram, x, COL8_848484, 59, y-23, 59, y-5);
    boxfill8(vram, x, COL8_000000, 2, y-3, 59, y-3);
    boxfill8(vram, x, COL8_000000, 60, y-24, 60, y-3);

    boxfill8(vram, x, COL8_848484, x-47, y-24, x-4, y-24);
    boxfill8(vram, x, COL8_848484, x-47, y-23, x-47, y-4);
    boxfill8(vram, x, COL8_FFFFFF, x-47, y-3, x-4, y-3);
    boxfill8(vram, x, COL8_FFFFFF, x-3, y-24, x-3, y-3);
    return;
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
    int i;
    char *p, d;
    for (i = 0; i < 16; i++)
    {
        p = vram + (y + i) * xsize + x;
        d = font[i];
        if ((d & 0x80) != 0) { p[0] = c; }
        if ((d & 0x40) != 0) { p[1] = c; }
        if ((d & 0x20) != 0) { p[2] = c; }
        if ((d & 0x10) != 0) { p[3] = c; }
        if ((d & 0x08) != 0) { p[4] = c; }
        if ((d & 0x04) != 0) { p[5] = c; }
        if ((d & 0x02) != 0) { p[6] = c; }
        if ((d & 0x01) != 0) { p[7] = c; }
    }
    return;
    
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
    extern char halfwid[4096];
    for ( ; *s != 0x00; s++)
    {
        putfont8(vram, xsize, x, y, c, halfwid + *s * 16);
        x += 8;
    }
    return;
}

void init_mouse_cursor8(char *mouse, char bc)
{
    static char cursor[16][16] = {
        "**..............",
        "*O*.............",
        "*OO**...........",
        "*OOOO*..........",
        "*OOOOO**........",
        "*OOOOOOO*.......",
        "*OOOOOOOO**.....",
        "*OOOOOOOOOO*....",
        "*OOOOOOOOOOO**..",
        "*OOOOOOOOOOOOO*.",
        "*OOOO***********",
        "*OOO*...........",
        "*OO*............",
        "*O*.............",
        "**..............",
        "*..............."
    };
    int x, y;

    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            if (cursor[y][x] == '*') {
                mouse[y * 16 + x] = COL8_000000;
            }
            if (cursor[y][x] == 'O') {
                mouse[y * 16 + x] = COL8_FFFFFF;
            }
            if (cursor[y][x] == '.') {
                mouse[y * 16 + x] = bc;
            }
        }
    }
    return;
}

void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize)
{
    int x, y;
    for ( y = 0; y < pysize; y++)
    {
        for ( x = 0; x < pxsize; x++)
        {
            vram[(py0+y)*vxsize+(px0+x)] = buf[y*pxsize+x];
        }
    }
    return;
}

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
    boxfill8(sht->buf, sht->bxsize, b, x, y, x+l*8-1, y+15);
    putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
    sheet_refresh(sht, x, y, x+l*8, y+16);
    return;
}

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
    int x1=x0+sx, y1=y0+sy;
    boxfill8(sht->buf, sht->bxsize, COL8_848484, x0-2, y0-3, x1+1, y0-3);
    boxfill8(sht->buf, sht->bxsize, COL8_848484, x0-3, y0-3, x0-3, y1+1);
    boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0-3, y1+2, x1+1, y1+2);
    boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1+2, y0-3, x1+2, y1+2);
    boxfill8(sht->buf, sht->bxsize, COL8_000000, x0-1, y0-2, x1+0, y0-2);
    boxfill8(sht->buf, sht->bxsize, COL8_000000, x0-2, y0-2, x0-2, y1+0);
    boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0-2, y1+1, x1+0, y1+1);
    boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1+1, y0-2, x1+1, y1+1);
    boxfill8(sht->buf, sht->bxsize, c          , x0-1, y0-1, x1+0, y1+0);
    return;
}