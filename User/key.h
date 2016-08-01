#ifndef _KEY_H_
#define _KEY_H_

#define KEY1_PORT		GPIOC
#define KEY1_PIN		GPIO_Pin_2
#define KEY2_PORT		GPIOC
#define KEY2_PIN		GPIO_Pin_3
#define KEY_NR              2
#define KEY_LONG_PRESS_TM	30

typedef void (*key_press_handler_t)(void);
struct Key
{
    GPIO_TypeDef *port;
    uint16_t pin;
    key_press_handler_t press_hook, long_press_hook;
};
extern struct Key keys[KEY_NR];

void key_tick_hook(void);
void key_task_handler(void *pvParameters);

#endif
