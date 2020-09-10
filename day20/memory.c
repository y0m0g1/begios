#include "bootpack.h"

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

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
    unsigned int a;
    size = (size + 0xfff) & 0xfffff000;
    a = memman_alloc(man, size);
    return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
    int i;
    size = (size + 0xfff) & 0xfffff000;
    i = memman_free(man, addr, size);
    return i;
}