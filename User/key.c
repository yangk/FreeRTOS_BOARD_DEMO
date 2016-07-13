#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "stm32f10x_lib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "comfunc.h"
#include "key.h"
#include "smart_frame.h"
//---------------------------------------------------------------------------------------------
enum
{
    KEY_EVENT_NO,
	KEY_PRESS,
	KEY_LONG_PRESS,
};
struct KeyMessage
{
    int key_no;
    int key_event;
};
//---------------------------------------------------------------------------------------------
struct Key keys[KEY_NR] =
{
    {KEY1_PORT, KEY1_PIN, key1_hook, key1_long_hook},
    {KEY2_PORT, KEY2_PIN, key2_hook, key2_long_hook},
};
static QueueHandle_t keys_queue;
//---------------------------------------------------------------------------------------------
static inline int is_key_press(int key_no)
{
    if (key_no >= array_size(keys)) return 0; 

    return !(keys[key_no].port->IDR & keys[key_no].pin); 
}
//---------------------------------------------------------------------------------------------
static void setup_peri(struct Key *key)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin = key->pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(key->port, &GPIO_InitStructure);

    keys_queue = xQueueCreate(10, sizeof(struct KeyMessage));
}
//---------------------------------------------------------------------------------------------
static uint32_t key_pre;
static uint32_t key_press_cnt[array_size(keys)];
#define is_key_pre_press(key_no) tst_bit(key_pre, key_no)
#define set_key_pre_press(key_no) set_bit(key_pre, key_no)
#define reset_key_pre_press(key_no) clr_bit(key_pre, key_no)

void key_tick_hook(void)
{
    int i;

    static uint32_t key_status[array_size(keys)] = { 0 };

    for (i = 0; i < array_size(keys); ++i)
    {
        key_status[i] <<= 1;
        key_status[i] |= is_key_press(i);
    }
#define is_key_press_ok(key_no) ((key_status[key_no] & 0x03) == 0x03)
#define is_key_up_ok(key_no) ((key_status[key_no] & 0x03) == 0x00)

    for (i = 0; i < array_size(keys); ++i)
    {
        if (is_key_press_ok(i))
        {
            if (!is_key_pre_press(i)) //key down
            {
                key_press_cnt[i] = KEY_LONG_PRESS_TM;
                set_key_pre_press(i);
            } 
            else if (key_press_cnt[i] > 0)
            {
                if (--key_press_cnt[i] == 0) //over tm
                {
                    struct KeyMessage e;
                    e.key_no = i;
                    e.key_event = KEY_LONG_PRESS;
                    xQueueSend(keys_queue, &e, 0);
                }
            }
        } 
        else if (is_key_up_ok(i) && is_key_pre_press(i)) //key up
        {
            if (key_press_cnt[i] > 0)
            {
                struct KeyMessage e;
                e.key_no = i;
                e.key_event = KEY_PRESS;
                xQueueSend(keys_queue, &e, 0);
            }
            reset_key_pre_press(i);
        }
    }
}
//---------------------------------------------------------------------------------------------
void key_task_handler(void *pvParameters)
{
    struct KeyMessage message;
    struct Key *key = (struct Key*)pvParameters;

    setup_peri(key);

    for (;;)
    {
        while (xQueueReceive(keys_queue, &message, portMAX_DELAY) != pdPASS);

        if (message.key_event == KEY_PRESS && keys[message.key_no].press_hook) 
        {
            keys[message.key_no].press_hook();
        }
        else if (message.key_event == KEY_LONG_PRESS && keys[message.key_no].long_press_hook)
        {
            keys[message.key_no].long_press_hook();
        }
    }
}
//---------------------------------------------------------------------------------------------
