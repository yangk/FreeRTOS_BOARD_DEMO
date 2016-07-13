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

//---------------------------------------------------------------------------------------------
int uart_frame_opt(uint8_t buffer[], int len)
{
    return 0;
}
//---------------------------------------------------------------------------------------------
void key1_hook(void)
{
    printf("key1 hook\r\n");
    vTaskDelay(50/portTICK_RATE_MS);
}
//---------------------------------------------------------------------------------------------
void key2_hook(void)
{
    printf("key2 hook\r\n");
#if 0    
    uint8_t data[] = "flash test";
    uint8_t rvdata[50] = "fail test!!!!!\n";
    SPI_FLASH_SectorErase(0x00);
    flash_write(0X1000,data,sizeof(data));
    flash_read(0x00,rvdata,sizeof(data));
    printf("%s\r\n",rvdata);
#endif
    vTaskDelay(50/portTICK_RATE_MS);
}
//---------------------------------------------------------------------------------------------
void key1_long_hook(void)
{
    printf("key1 long hook\r\n");
    vTaskDelay(50/portTICK_RATE_MS);
}
//---------------------------------------------------------------------------------------------
void key2_long_hook(void)
{
    printf("key2 long hook\r\n");
    vTaskDelay(50/portTICK_RATE_MS);
}
//---------------------------------------------------------------------------------------------
 void vApplicationIdleHook( void )
 {
    for( ;; )
    {
        printf("idle_hook!!\r\n");
    }
 }
//---------------------------------------------------------------------------------------------
void vApplicationMallocFailedHook( void )
{
    for( ;; )
    {
        printf("malloc failed!!\r\n");
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