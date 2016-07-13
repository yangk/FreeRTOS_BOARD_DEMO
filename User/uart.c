#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "stm32f10x_lib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "comfunc.h"
#include "uart.h"
#include "smart_frame.h"

//---------------------------------------------------------------------------------------------
struct UartPeri uart_peri[] = 
{
    {USART2, RCC_APB1Periph_USART2, USART2_IRQn, {GPIOA, GPIO_Pin_2},  {GPIOA, GPIO_Pin_3}},
    {USART3, RCC_APB1Periph_USART3, USART3_IRQn, {GPIOB, GPIO_Pin_10}, {GPIOB, GPIO_Pin_11}},
    //{UART4,  RCC_APB1Periph_UART4,  UART4_IRQn,  {GPIOC, GPIO_Pin_10}, {GPIOC, GPIO_Pin_11}},
    //{UART5,  RCC_APB1Periph_UART5,  UART5_IRQn,  {GPIOC, GPIO_Pin_12}, {GPIOD, GPIO_Pin_2}},
};
//---------------------------------------------------------------------------------------------
struct UartSetting uart_setting[] =
{
    {9600, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No},
    {9600, USART_WordLength_9b, USART_StopBits_1, USART_Parity_Even},//645
};
//---------------------------------------------------------------------------------------------
struct UartPort ports[UART_PORT_NR] = 
{
    {&uart_peri[0], &uart_setting[0], {NULL, 0x100}, {NULL, 0x100}, 0, NULL, NULL},
    {&uart_peri[1], &uart_setting[0], {NULL, 0x100}, {NULL, 0x100}, 0, NULL, uart_frame_opt},
    //{&uart_peri[2], &uart_setting[0], {NULL, 0x100}, {NULL, 0x100}, 0, NULL, NULL},
    //{&uart_peri[3], &uart_setting[0], {NULL, 0x100}, {NULL, 0x100}, 0, NULL, NULL},
};
//---------------------------------------------------------------------------------------------
static void setup_uart_port(const struct GpioPin *tx, const struct GpioPin *rx)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure USARTx_Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = tx->pin; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(tx->port, &GPIO_InitStructure); 

    /* Configure USARTx_Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = rx->pin; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(rx->port, &GPIO_InitStructure); 
}

static void setup_uart_setting(const struct UartPeri *peri, const struct UartSetting *setting)
{
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate = setting->baud; 
    USART_InitStructure.USART_WordLength = setting->databits; 
    USART_InitStructure.USART_StopBits = setting->stopbits; 
    USART_InitStructure.USART_Parity = setting->parity; 
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure the USARTx */
    USART_Init(peri->uart, &USART_InitStructure); 
}
//---------------------------------------------------------------------------------------------
static void setup_uart(struct UartPort *port)
{
    port->cache_tx.queue = xQueueCreate(port->cache_tx.queue_length, sizeof(uint8_t)); 
    port->cache_rx.queue = xQueueCreate(port->cache_rx.queue_length, sizeof(uint8_t)); 
    vSemaphoreCreateBinary(port->sem); 
    xSemaphoreTake(port->sem, 0);

    RCC_APB1PeriphClockCmd(port->peri->periph, ENABLE); 

    setup_uart_port(&port->peri->tx, &port->peri->rx);
    setup_uart_setting(port->peri, port->setting);

    /* Enable USART2 Receive and Transmit interrupts */
    USART_ITConfig(port->peri->uart, USART_IT_RXNE, ENABLE);
    USART_ITConfig(port->peri->uart, USART_IT_TXE, DISABLE);

    NVIC_EnableIRQ(port->peri->irq); 

    /* Enable the USARTx */
    USART_Cmd(port->peri->uart, ENABLE);
}
//---------------------------------------------------------------------------------------------
void uart_send(struct UartPort *port, const void *data, int len)
{
    for (int i = 0; i < len; i++)
    {
        xQueueSend(port->cache_tx.queue, &((const uint8_t*)data)[i], portMAX_DELAY);
    }
    USART_ITConfig(port->peri->uart, USART_IT_TXE, ENABLE);
}
//---------------------------------------------------------------------------------------------
static void uart_rx_hook(struct UartPort *port)
{
    port->over_time_tick = UART_OVER_TM_MAX;
}
void uart_20ms_hook(void)
{
    for (int i = 0; i < array_size(ports); i++)
    {
        struct UartPort *port = &ports[i];

        if (port->over_time_tick > 0)
        {
            if (--port->over_time_tick == 0)
            {
                xSemaphoreGiveFromISR(port->sem, NULL); 
            }
        }
    }
}
//---------------------------------------------------------------------------------------------
void uart_interrupt_handler(struct UartPort *port)
{
    uint8_t c;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    USART_TypeDef *uart = port->peri->uart;

    if (USART_GetITStatus(uart, USART_IT_TXE) == SET) 
    {
        if (xQueueReceiveFromISR(port->cache_tx.queue, &c, NULL) == pdPASS) 
        {
            USART_SendData(uart, c);
        }
        else
        {
            USART_ITConfig(uart, USART_IT_TXE, DISABLE);
        }
    }

    if (USART_GetITStatus(uart, USART_IT_RXNE))
    {
        c = USART_ReceiveData(uart);
        xQueueSendFromISR(port->cache_rx.queue, &c, &xHigherPriorityTaskWoken); 
        uart_rx_hook(port);
    }

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

//---------------------------------------------------------------------------------------------
void uart_task_handler(void *args)
{
    int len = 0;
    uint8_t cache[0x100];
    struct UartPort *port = (struct UartPort*)args;

    setup_uart(port);

    for (;;)
    {
        if (xSemaphoreTake(port->sem, portMAX_DELAY) == pdPASS)
        {
            while (xQueueReceive(port->cache_rx.queue, &cache[len++], 0) == pdPASS); 
            
            if (port->uart_frame_hook) 
            {
                port->uart_frame_hook(cache, len);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------

