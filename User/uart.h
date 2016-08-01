#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#define UART_PORT_NR        2
#define UART_OVER_TM_MAX    10

struct GpioPin
{
    GPIO_TypeDef *port;
    uint16_t pin;
};
struct UartPeri
{
    USART_TypeDef *uart;
    uint32_t periph;
    enum IRQn irq;

    struct GpioPin tx, rx;
};
struct UartSetting
{
    uint32_t baud;
    uint16_t databits, stopbits, parity;
};
struct UartCache
{
    QueueHandle_t queue;
    uint32_t queue_length;
};
struct UartPort
{
    struct UartPeri *peri;
    struct UartSetting *setting;

    struct UartCache cache_tx, cache_rx;
    int over_time_tick;
    SemaphoreHandle_t sem;

    int (*uart_frame_hook)(uint8_t *in, int len);
};
extern struct UartPort ports[UART_PORT_NR];

void uart_interrupt_handler(struct UartPort *port);
static inline void uart2_interrupt_handler(void)
{
    uart_interrupt_handler(&ports[0]);
}
static inline void uart3_interrupt_handler(void)
{
    uart_interrupt_handler(&ports[1]);
}

void uart_send(struct UartPort *port, const void *data, int len);
void uart_20ms_hook(void);
void uart_task_handler(void *args);

#endif
