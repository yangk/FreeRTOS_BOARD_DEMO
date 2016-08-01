#include <stdio.h>
#include "string.h"
#include "stm32f10x_lib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "comfunc.h"
#include "smart_frame.h"
#include "spiflash.h"
#include "dev_ctrl.h"
#include "debug.h"

#define frame_opt_log(M, ...) custom_log("frame_opt", M, ##__VA_ARGS__)
#define frame_opt_log_trace() custom_log_trace("frame_opt")
//---------------------------------------------------------------------------------------------
int uart_frame_opt(uint8_t buffer[], int len)
{
    return 0;
}
//---------------------------------------------------------------------------------------------
void key1_hook(void)
{
    frame_opt_log_trace();
    //frame_opt_log("key1 hook");
    vTaskDelay(50/portTICK_RATE_MS);
}
//---------------------------------------------------------------------------------------------
void key2_hook(void)
{
    frame_opt_log_trace();
    //frame_opt_log("key2 hook");
    vTaskDelay(50/portTICK_RATE_MS);
}
//---------------------------------------------------------------------------------------------
void key1_long_hook(void)
{
    frame_opt_log_trace();
    //frame_opt_log("key1 long hook");
    vTaskDelay(50/portTICK_RATE_MS);
}
//---------------------------------------------------------------------------------------------
void key2_long_hook(void)
{
    frame_opt_log_trace();
    vTaskDelay(50/portTICK_RATE_MS);
    frame_opt_log("key2 long hook");
    vTaskDelay(50/portTICK_RATE_MS);
}
//---------------------------------------------------------------------------------------------
 void vApplicationIdleHook( void )
 {
    for( ;; )
    {
        frame_opt_log("idle hook");
    }
 }
//---------------------------------------------------------------------------------------------
void vApplicationMallocFailedHook( void )
{
    for( ;; )
    {
        frame_opt_log("malloc failed");
    }
}
//---------------------------------------------------------------------------------------------
void vApplicationTickHook( void )
{
    for( ;; )
    {
        LED_TOGGLE(LED1_PORT,LED1_PIN);
    }
}