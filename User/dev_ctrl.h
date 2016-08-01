#ifndef _DEV_CTRL_H_
#define _DEV_CTRL_H_

#define MAIN_CLK            (8000000)
#define SYS_MAIN_CLK	    (72000000)

#define LED1_PORT   GPIOA
#define LED1_PIN	GPIO_Pin_1
#define	LED1_SET()	GPIO_SetBits(LED1_PORT,LED1_PIN)
#define	LED1_CLR()	GPIO_ResetBits(LED1_PORT,LED1_PIN)
#define LED2_PORT   GPIOA
#define LED2_PIN	GPIO_Pin_2
#define	LED2_SET()	GPIO_SetBits(LED2_PORT,LED2_PIN)
#define	LED2_CLR()	GPIO_ResetBits(LED2_PORT,LED2_PIN)

#define LED_TOGGLE(LED_PORT,LED_PIN)  LED_PORT->ODR ^= LED_PIN;

void setup_rcc(void);
void init_tim2(void);
void gpio_init(void);
void modules_init(void);
#endif
