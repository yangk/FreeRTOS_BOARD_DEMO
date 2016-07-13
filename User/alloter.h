#ifndef _ALLOTER_H_
#define _ALLOTER_H_

#include <stdint.h>

#define BUF_SZ	0x200


struct _CHN_SLOT
{
    int tx, rx, data_cnt, data_max;
};
#define BLK_NO_SHIFT  	6
#define BLK_SZ     		(0x01 << BLK_NO_SHIFT)

#define INVALID_BLK_NO  0xFF
#define INVALID_PTR     (INVALID_BLK_NO << BLK_NO_SHIFT)

void init_chn_pool_mgr(void);
int put_chn_bytes(struct _CHN_SLOT *pCHN_SLOT, const uint8_t buffer[], int len);
int get_chn_bytes(struct _CHN_SLOT *pCHN_SLOT, uint8_t buffer[], int len);
int peek_chn_bytes(struct _CHN_SLOT *pCHN_SLOT, uint8_t data[], int len);
#endif
