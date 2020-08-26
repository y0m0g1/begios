#include <stdio.h>
#include "bootpack.h"

extern struct FIFO8 keyfifo, mousefifo;

// main function
void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    char s[40], mcursor[256], keybuf[32], mousebuf[128];
    int mx, my, i;
    unsigned int memtotal;
    struct MOUSE_DEC mdec;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

    init_gdtidt();
    init_pic();
    io_sti();
    fifo8_init(&keyfifo, 32, keybuf);
    fifo8_init(&mousefifo, 128, mousebuf);
    io_out8(PIC0_IMR, 0xf9);
    io_out8(PIC1_IMR, 0xef);

    init_keyboard();
    enable_mouse(&mdec);
    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal-0x00400000);
    
    init_palette();
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
    sprintf(s, "(%d, %d)", mx, my);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

    sprintf(s, "memory:%dMB    free:%dKB", memtotal/(1024*1024), memman_total(memman)/1024);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

    for (;;) 
    {
        io_cli();
        if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0)
        {
            io_stihlt();
        }
        else
        {
            if(fifo8_status(&keyfifo) != 0)
            {
                i = fifo8_get(&keyfifo);            
                io_sti();
                sprintf(s, "%02X", i);
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
            }
            else if(fifo8_status(&mousefifo) != 0)
            {
                i = fifo8_get(&mousefifo);            
                io_sti();
                if (mouse_decode(&mdec, i) != 0)
                {
                    sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
                    if ((mdec.btn & 0x01) != 0)
                    {
                        s[1] = 'L';
                    }
                    if ((mdec.btn & 0x02) != 0)
                    {
                        s[3] = 'R';
                    }
                    if ((mdec.btn & 0x04) != 0)
                    {
                        s[2] = 'C';
                    }
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32+15*8-1, 31);
                    putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
                    // move mouse cursor
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx+15, my+15); // delete cursor
                    mx += mdec.x;
                    my += mdec.y;
                    if (mx < 0)
                    {
                        mx = 0;
                    }
                    if (my < 0)
                    {
                        my = 0;
                    }
                    if (mx > binfo->scrnx - 16)
                    {
                        mx = binfo->scrnx - 16;
                    }
                    if (my > binfo->scrny - 16)
                    {
                        my = binfo->scrny - 16;
                    }
                    sprintf(s, "(%3d, %3d)", mx, my);
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
                    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
                    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
                }
            }
        }
        
    }
}

unsigned int memtest(unsigned int start, unsigned int end)
{
    char flag486 = 0;
    unsigned int eflag, cr0, i;

    eflag = io_load_eflags();
    eflag |= EFLAGS_AC_BIT;
    io_store_eflags(eflag);
    eflag = io_load_eflags();
    if ((eflag & EFLAGS_AC_BIT) != 0) // if 386 ac bit will be 0 automatically 
    {
        flag486 = 1;
    }
    eflag &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflag);
    if (flag486 != 0)
    {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE; // cache disable
        store_cr0(cr0);
    }
    i = memtest_sub(start, end);
    if (flag486 != 0)
    {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE; // cache enable
        store_cr0(cr0);
    }

    return i;
}

/* initialize memory manager */
void memman_init(struct MEMMAN *man)
{
    man->frees = 0;     
    man->maxfrees = 0;
    man->lostsize = 0;  // sum-size which failed to free
    man->losts = 0;     // counts failed to free
    return;
}

/* return the total free size */
unsigned int memman_total(struct MEMMAN *man)
{
    unsigned int i, t=0;
    for ( i = 0; i < man->frees; i++)
    {
        t += man->free[i].size;
    }
    return t;
}

/* memory allocate */
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
    unsigned int i, a;
    for ( i = 0; i < man->frees; i++)
    {
        if (man->free[i].size >= size)
        {
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            if (man->free[i].size == 0)
            {
                // move over
                man->frees--;
                for ( ; i < man->frees; i++)
                {
                    man->free[i] = man->free[i+1];
                }
            }
            return a;
        }
    }
    return 0; // no space
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
    int i,j;
    for ( i = 0; i < man->frees; i++)
    {
        if (man->free[i].addr > addr)
        {
            break;
        }
        
    }
    // free[i-1].addr < addr < free[i].addr
    if (i > 0)
    {
        // integrate with the previous
        if (man->free[i-1].addr + man->free[i-1].size == addr)
        {
            man->free[i-1].size += size;
            if (i < man->frees)
            {
                // also integrate with the next
                if (addr + size == man->free[i].addr)
                {
                    man->free[i-1].size += man->free[i].size;
                    man->frees--;
                    for ( ; i < man->frees; i++)
                    {
                        man->free[i] = man->free[i+1];
                    }
                    
                }
                
            }
            return 0; // succeed
        }
    }
    // cannot integrate with the previous
    if (i < man->frees)
    {
        // integrate with the next free space (not the previous one)
        if (addr + size == man->free[i].addr)
        {
            man->free[i].addr = addr;
            man->free[i].size += size;
            return 0; // succeed
        }
    }
    // cannot integrate with the prev/next
    if (man->frees < MEMMAN_FREES)
    {
        // shift to the back
        for ( j = man->frees; j > i; j--)
        {
            man->free[j] = man->free[j-1];
        }
        man->frees++;
        if (man->maxfrees < man->frees)
        {
            man->maxfrees = man->frees;
        }
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0; // succeed
    }
    // failed to shift
    man->losts++;
    man->lostsize += size;
    return -1; // failed
    
}