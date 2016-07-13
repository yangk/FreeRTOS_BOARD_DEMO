#include <stdio.h>
#include "stm32f10x.h"
#include "stm32f10x_lib.h"
#include "FreeRTOS.h"
#include "comfunc.h"
#include "task.h"
#include "key.h"
#include "debug.h"
#include "task_timer.h"
#include "uart.h"
#include "dev_ctrl.h"
#include "smart_frame.h"

//---------------------------------------------------------------------------------------------
#define KEY_TASK_PRIORITY			(configMAX_PRIORITIES - 2)
#define KEY_TASK_STACK_SIZE			(configMINIMAL_STACK_SIZE + 50)

#define DEBUG_PORT_TASK_PRIORITY	(configMAX_PRIORITIES - 1)
#define DEBUG_PORT_TASK_STACK_SIZE	(configMINIMAL_STACK_SIZE + 50)

#define UART_TASK_PRIORITY	        (configMAX_PRIORITIES - 1)
#define UART_TASK_STACK_SIZE	    (configMINIMAL_STACK_SIZE + 0x150)

#define DEFAULT_TASK_PRIORITY	    (configMAX_PRIORITIES - 1)
#define DEFAULT_TASK_STACK_SIZE	    (configMINIMAL_STACK_SIZE + 0x50)
//---------------------------------------------------------------------------------------------
int main(void)
{
    modules_init();
    
    xTaskCreate(debug_port_task_handler, "debug", DEBUG_PORT_TASK_STACK_SIZE, NULL, DEBUG_PORT_TASK_PRIORITY, NULL);
    xTaskCreate(uart_task_handler, "uart2", UART_TASK_STACK_SIZE, &ports[0], UART_TASK_PRIORITY, NULL);
    xTaskCreate(uart_task_handler, "uart3", UART_TASK_STACK_SIZE, &ports[1], UART_TASK_PRIORITY, NULL);
    
    for (int i = 0; i < array_size(timers); ++i)
    {
        timers[i].timer = xTimerCreate(NULL, timers[i].period, pdTRUE, NULL, timers[i].handler);
        xTimerStart(timers[i].timer, portMAX_DELAY);
    }
    xTaskCreate(key_task_handler, "key1", KEY_TASK_STACK_SIZE, &keys[0], KEY_TASK_PRIORITY, NULL);
    xTaskCreate(key_task_handler, "key2", KEY_TASK_STACK_SIZE, &keys[1], KEY_TASK_PRIORITY, NULL);

    vTaskStartScheduler();

    return 0;
}
//---------------------------------------------------------------------------------------------
