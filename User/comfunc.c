#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "stm32f10x_lib.h"
#include "comfunc.h"

//---------------------------------------------------------------------------------------
/* 1.21us in 72MHz */
void delayus(unsigned long us)
{
    us *= 8;
    while (us--)
        __NOP();
}

void delayms(unsigned long ms)
{
    while (ms--)
        delayus(1000);
}
//---------------------------------------------------------------------------------------
uint8_t checksum(const void *data, int len)
{
    uint8_t cs = 0, *p = (uint8_t *) data;

    while (len-- > 0)
        cs += *p++;
    return cs;
}
//---------------------------------------------------------------------------------------
void reverse(void *buff, int len)
{
    uint8_t *rearp = (uint8_t *) buff + len - 1;
    uint8_t *headp = (uint8_t *) buff;

    if (len < 1) return;
    while (headp < rearp)
    {
        swap(*headp, *rearp);
        headp++;
        rearp--;
    }
}
//---------------------------------------------------------------------------------------
int get_last_bit_seqno(uint32_t x)
{
    int n = 0;

    if (x == 0) return -1;
    while (!tst_bit(x, n)) n++;

    return n;
}
//---------------------------------------------------------------------------------------
int is_all_xx(const void *arr, uint8_t val, int n)
{
    while (n && *(uint8_t *) arr == val)
    {
        arr = (uint8_t *) arr + 1;
        n--;
    }
    return !n;
}
int is_all_bcd(const void *arr, int n)
{
    const uint8_t *_arr = (const uint8_t *)arr;

    while (n--)
    {
        if ((_arr[n] & 0x0F) > 0x09 || (_arr[n] & 0xF0) > 0x90)
            return 0;
    }
    return 1;
}
//---------------------------------------------------------------------------------------
void memaddnum(uint8_t mem[], int num, int cnt)
{
    while (cnt--) mem[cnt] += num;
}
//---------------------------------------------------------------------------------------
u32 get_le_val(const uint8_t * p, int bytes)
{
    u32 ret = 0;

    while (bytes-- > 0)
    {
        ret <<= 8;
        ret |= *(p + bytes);
    }
    return ret;
}
uint32_t get_be_val(const uint8_t * p, int bytes)
{
    uint32_t ret = 0;
    while (bytes-- > 0)
    {
        ret <<= 8;
        ret |= *p++;
    }

    return ret;
}
void put_le_val(u32 val, uint8_t * p, int bytes)
{
    while (bytes-- > 0)
    {
        *p++ = val & 0xFF;
        val >>= 8;
    }
}
void put_be_val(uint32_t val, uint8_t * p, int bytes)
{
    while (bytes-- > 0)
    {
        *(p + bytes) = val & 0xFF;
        val >>= 8;
    }
}
//---------------------------------------------------------------------------------------
static const uint32_t _power10[] = {1,10,100,1000,10000,100000,1000000,10000000};
void float2bin(float f, int ibits, int fbits, uint8_t bin[])
{
    if (f < 0.0) f *= -1.0;

    put_le_val((int)(f * _power10[fbits] + 0.5), bin, ibits);
}
void float2bcd(float f, int ibits, int fbits, uint8_t bcd[])
{
    int i;
    uint32_t val;

    if (f < 0.0) f *= -1.0;

    val = (int)(f * _power10[fbits] + 0.5);
    for (i = 0; i < ibits; i++)
    {
        bcd[i] = bin2bcd(val % 100);
        val /= 100;
    }
}
float bin2float(uint8_t bin[], int ibits, int fbits)
{
    uint32_t val = 0;

    while (ibits--)
    {
        val = (val << 8) | bin[ibits];
    }
    return (float)val/_power10[fbits];
}
float bcd2float(uint8_t bcd[], int ibits, int fbits)
{
    float val = 0.0;

    while (ibits--)
    {
        val = val * 100 + bcd2bin(bcd[ibits]);
    }
    return val/_power10[fbits];
}
//---------------------------------------------------------------------------------------
uint32_t bcd2int(uint8_t bcd[], int n)
{
    uint32_t val = 0;

    while (n--)
    {
        val = val * 100 + bcd2bin(bcd[n]);
    }
    return val;
}
void int2bcd(uint32_t val, uint8_t bcd[], int n)
{
    int i;

    for (i = 0; i < n; i++)
    {
        bcd[i] = bin2bcd(val % 100);
        val /= 100;
    }
}
//---------------------------------------------------------------------------------------
static uint16_t do_get_crc16(uint32_t crc, uint8_t byte)
{
    uint32_t in = byte | 0x100;

    do
    {
        crc <<= 1;
        in <<= 1;
        if (in & 0x100)
            ++crc;
        if (crc & 0x10000)
            crc ^= 0x1021;
    }
    while (!(in & 0x10000));

    return crc & 0xFFFFu;
}
uint16_t get_crc16(const void *data, uint32_t size)
{
    uint32_t crc = 0;
    const uint8_t *_data = (uint8_t *) data;
    const uint8_t *dataEnd = _data + size;

    while (_data < dataEnd)
        crc = do_get_crc16(crc, *_data++);
    crc = do_get_crc16(crc, 0);
    crc = do_get_crc16(crc, 0);

    return crc & 0xFFFFu;
}
//---------------------------------------------------------------------------------------

