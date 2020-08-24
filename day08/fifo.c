#include "bootpack.h"

/* intialize fifo buffer */
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf)
{
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->p = 0; // read label
    fifo->q = 0; // write label
    return;
}

/* put and store a data in fifo buffer */
int fifo8_put(struct FIFO8 *fifo, unsigned char data)
{
    if (fifo->free == 0)
    {
        fifo->flags |= FLAGS_OVERRUN; // overruned because of no free space
        return -1;
    }
    fifo->buf[fifo->p++] = data;
    if (fifo->p == fifo->size)
    {
        fifo->p = 0;
    }
    fifo->free--;
    return 0;
    
}

/* get a data from fifo buffer */
int fifo8_get(struct FIFO8 *fifo)
{
    int data;
    if (fifo->free == fifo->size)
    {
        // if empty, retun -1
        return -1;
    }
    data = fifo->buf[fifo->q++];
    if (fifo->q == fifo->size)
    {
        fifo->q = 0;
    }
    fifo->free++;
    return data;    
}

// return how much data are stored
int fifo8_status(struct FIFO8 *fifo)
{
    return fifo->size - fifo->free;
}