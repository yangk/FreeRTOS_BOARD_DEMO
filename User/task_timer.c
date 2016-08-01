#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "stm32f10x_lib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "comfunc.h"
#include "key.h"
#include "uart.h"
#include "task_timer.h"



//---------------------------------------------------------------------------------------------
void ms20_handler(TimerHandle_t xTimer)
{
    key_tick_hook();
    uart_20ms_hook();
}
//---------------------------------------------------------------------------------------------
void ms100_handler(TimerHandle_t xTimer)
{

}
//---------------------------------------------------------------------------------------------
void sec1_handler(TimerHandle_t xTimer)
{

}
//---------------------------------------------------------------------------------------------
struct TaskTimer timers[TIMER_NR] = 
{
    {NULL, 20/portTICK_PERIOD_MS,   ms20_handler},
    {NULL, 100/portTICK_PERIOD_MS,  ms100_handler},
    {NULL, 1000/portTICK_PERIOD_MS, sec1_handler},
};
//---------------------------------------------------------------------------------------------

