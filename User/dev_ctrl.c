#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "stm32f10x_lib.h"
#include "key.h"
#include "dev_ctrl.h"
#include "spiflash.h"

//---------------------------------------------------------------------------------------------
void setup_rcc(void)
{
    ErrorStatus HSEStartUpStatus;

    RCC_DeInit();

    RCC_HSEConfig(RCC_HSE_ON);

    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if (HSEStartUpStatus == SUCCESS)
    {
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
        FLASH_SetLatency(FLASH_Latency_2);
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        RCC_PCLK2Config(RCC_HCLK_Div1);
        RCC_PCLK1Config(RCC_HCLK_Div2);
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);//此部分用来调CPU的主频
        RCC_PLLCmd(ENABLE);
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        while (RCC_GetSYSCLKSource() != 0x08);
    }
}
//---------------------------------------------------------------------------------------------
void init_tim2(void)//1khz
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    TIM_DeInit(TIM2);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseStructure.TIM_Period=9999;//ARR的值
    TIM_TimeBaseStructure.TIM_Prescaler=(uint16_t)(SYS_MAIN_CLK/1000000-1);
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM2, ENABLE); //开启时钟

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);   // 抢占式优先级别 
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;//指定中断源
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;// 指定响应优先级别1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
//---------------------------------------------------------------------------------------------
void gpio_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* LED0 GPIO */
    gpio_init_struct.GPIO_Pin = LED1_PIN;
    gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED1_PORT, &gpio_init_struct);

    /* LED1 GPIO */
    gpio_init_struct.GPIO_Pin = LED2_PIN;
    GPIO_Init(LED2_PORT, &gpio_init_struct);
    
    LED1_SET();
    LED2_SET();
}
//---------------------------------------------------------------------------------------------
void modules_init(void)
{
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x2000);
    setup_rcc();
    init_tim2();
    gpio_init();
    sys_spi_init();
}
//---------------------------------------------------------------------------------------------