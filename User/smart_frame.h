#ifndef __SMART_FRAME__
#define __SMART_FRAME__
#include "stm32f10x.h"
#include "uart.h"

#define STC             0x7E
#define ID_LEN          4
#define PD_LEN          2
#define MAC_LEN         6

#define CMD_SET_AID     0x01
#define CMD_GET_EID     0x07
#define CMD_ACK_EID     0x17
#define CMD_ACK         0x00
#define CMD_NAK         0xFF
#define CMD_READ_REV    0x04

#pragma pack(1)
struct Body
{
    u8 did[2];
    u8 ctrl;
    u8 data[1];
};
#define FBD_FRAME_HEAD   offsetof(struct Body, data)

struct SmartFrame
{
    u8 stc;
    u8 said[ID_LEN];
    u8 taid[ID_LEN];
    u8 seq;
    u8 len;
    u8 data[1];
};

struct frame645
{
    u8 frame_start_flag1;
    u8 addr[6];
    u8 frame_start_flag2;
    union
    {
        u8 control_byte;
        struct
        {
            u8 function_flag:5;
            u8 following_flag:1;
            u8 exception_flag:1;
            u8 direction_flag:1;
        } control_bits;
    } control_code;
    u8 datalen;
    u8 data[1];
};

#define SMART_FRAME_HEAD offsetof(struct SmartFrame, data)

struct AppFrame
{
    u8 cmd;
    u8 data[1];
};

struct Flag
{
    u8 aid_time;
    u8 test_time;
    u8 try_cnt_aid;
    u8 try_cnt_test;
    u8 end_time;
};
#pragma pack()
int uart_frame_opt(uint8_t buffer[], int len);
void key1_hook(void);
void key2_hook(void);
void key1_long_hook(void);
void key2_long_hook(void);
#endif

