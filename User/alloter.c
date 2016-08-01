#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "comfunc.h"
#include "alloter.h"


struct _CHN_POOL_MGR
{
	uint8_t buffer[BUF_SZ];
	uint32_t free_bitmap;
};
struct _CHN_POOL_MGR chn_pool_mgr;

void init_chn_pool_mgr(void)
{ 
    while ((sizeof(chn_pool_mgr.buffer) / BLK_SZ) > 8*sizeof(chn_pool_mgr.free_bitmap));
    chn_pool_mgr.free_bitmap = (1 << (sizeof(chn_pool_mgr.buffer) / BLK_SZ)) -1;
}

static uint32_t alloc_a_slot(void)
{
    int k;
    uint32_t bitmap;

    bitmap = chn_pool_mgr.free_bitmap ;
    if (0x00 == bitmap )
        return(INVALID_BLK_NO);
    k = get_last_bit_seqno(bitmap);
    bitmap &= (bitmap -1);
    chn_pool_mgr.free_bitmap = bitmap;
    chn_pool_mgr.buffer[(k << BLK_NO_SHIFT)| (BLK_SZ - 1)] = INVALID_BLK_NO;
    return(k);
}

static void free_a_slot(int blk_no)
{
    chn_pool_mgr.free_bitmap |= 1 << blk_no ;
}

int put_chn_bytes(struct _CHN_SLOT *pCHN_SLOT, const uint8_t buffer[] ,int len)
{ 
    int k,i = 0;

    if (len < 1)  return(0);
    portENTER_CRITICAL();
    while (len > 0x00)
    {
        if (pCHN_SLOT->data_cnt > pCHN_SLOT->data_max )
        {
            //over limit 
            goto put_chn_exit;
        }
        if (INVALID_BLK_NO == (pCHN_SLOT->rx >> BLK_NO_SHIFT))
        {
            //not get any buffer
            pCHN_SLOT->tx = pCHN_SLOT->rx = alloc_a_slot() << BLK_NO_SHIFT;
            if (INVALID_BLK_NO == (pCHN_SLOT->rx >> BLK_NO_SHIFT))
            {
                //no room
                pCHN_SLOT->tx = pCHN_SLOT->rx = INVALID_BLK_NO << BLK_NO_SHIFT;
                pCHN_SLOT->data_cnt = 0x00;

                goto put_chn_exit;
            }
        }
        if ((BLK_SZ - 1) == (pCHN_SLOT->rx & (BLK_SZ - 1)))
        {
            chn_pool_mgr.buffer[pCHN_SLOT->rx] = alloc_a_slot(); 
            if (INVALID_BLK_NO == chn_pool_mgr.buffer[pCHN_SLOT->rx]  )
            { //no room 
                goto put_chn_exit;
            }
            pCHN_SLOT->rx = chn_pool_mgr.buffer[pCHN_SLOT->rx] << BLK_NO_SHIFT;
        }

        //some space free
        k = pCHN_SLOT->rx & (BLK_SZ - 1);
        k = (BLK_SZ - 1) - k;
        k = min(len,k);
        memcpy(&chn_pool_mgr.buffer[pCHN_SLOT->rx],&buffer[i],k);
        len -= k;
        pCHN_SLOT->rx += k;
        i += k; 
    }
put_chn_exit:
    pCHN_SLOT->data_cnt += i;
    portEXIT_CRITICAL();
    return(i);
}

int peek_chn_bytes(struct _CHN_SLOT *pCHN_SLOT,uint8_t data[],int len)
{ 
    int n = 0;
    struct _CHN_SLOT chn_slot;

    portENTER_CRITICAL();
    chn_slot.rx = pCHN_SLOT->rx;
    chn_slot.tx = pCHN_SLOT->tx;
    chn_slot.data_cnt = pCHN_SLOT->data_cnt;
    portEXIT_CRITICAL();

    while ((len > 0) && (chn_slot.tx != chn_slot.rx))
    {
        if ((BLK_SZ - 1) == (chn_slot.tx & (BLK_SZ - 1)))
        {
            chn_slot.tx = chn_pool_mgr.buffer[chn_slot.tx] << BLK_NO_SHIFT;
        }
        data[n++] = chn_pool_mgr.buffer[chn_slot.tx];
        chn_slot.tx++;
        len--;    
    }  
    return(n);
}

int get_chn_bytes(struct _CHN_SLOT *pCHN_SLOT, uint8_t buffer[], int len)
{
    int k,i = 0x00;
    if (len < 0x01) return(0);

    portENTER_CRITICAL();
    while (len > 0x00)
    {
        if (0x00000 == pCHN_SLOT->data_cnt)
        {
            pCHN_SLOT->tx = pCHN_SLOT->rx = INVALID_BLK_NO << BLK_NO_SHIFT;
            goto get_chn_exit;
        }
        if ((BLK_SZ - 1) == (pCHN_SLOT->tx  & (BLK_SZ - 1)))
        {
            free_a_slot(pCHN_SLOT->tx >> BLK_NO_SHIFT);
            if (INVALID_BLK_NO ==chn_pool_mgr.buffer[pCHN_SLOT->tx] )
            {
                pCHN_SLOT->tx = pCHN_SLOT->rx = INVALID_BLK_NO << BLK_NO_SHIFT;
                if (0x00 != pCHN_SLOT->data_cnt)
                {
                    while (1);
                }
                pCHN_SLOT->data_cnt = 0x00;
                goto get_chn_exit;
            }
            pCHN_SLOT->tx = chn_pool_mgr.buffer[pCHN_SLOT->tx] << BLK_NO_SHIFT;
        }
        //some space free
        k = pCHN_SLOT->tx & (BLK_SZ - 1);
        k = (BLK_SZ - 1) - k; 
        k = min(len ,k);
        k = min(k ,pCHN_SLOT->data_cnt );
        memcpy(&buffer[i],&chn_pool_mgr.buffer[pCHN_SLOT->tx],k);
        len -= k;
        pCHN_SLOT->tx += k;
        i += k;
        pCHN_SLOT->data_cnt -= k;                         
        if ( pCHN_SLOT->data_cnt <= 0x00)
        {
            free_a_slot(pCHN_SLOT->tx >> BLK_NO_SHIFT);
            pCHN_SLOT->tx = pCHN_SLOT->rx = INVALID_BLK_NO << BLK_NO_SHIFT;
            goto get_chn_exit;
        }
    }
get_chn_exit: 
    portEXIT_CRITICAL();
    return(i);
}
