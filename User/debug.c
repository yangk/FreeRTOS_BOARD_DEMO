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
#include "debug.h"
#include "dev_ctrl.h"
#include "core_cm3.h"

//---------------------------------------------------------------------------------------------
static const char PROMPT[] = "dBUG> ";

//---------------------------------------------------------------------------------------------
struct cmd_arg
{
    int len;
    char cmdline[UIF_MAX_LINE];
};
static struct cmd_arg cmd_args;

static QueueHandle_t txchars_queue;
static SemaphoreHandle_t xRxCompleteSemaphore = NULL;
//---------------------------------------------------------------------------------------------
static void setup_debug_port_peri(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    txchars_queue = xQueueCreate(100, sizeof(signed char));
    vSemaphoreCreateBinary(xRxCompleteSemaphore);
    xSemaphoreTake(xRxCompleteSemaphore, 0);

    /* Enable USART1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    /* Configure USART1 Rx (PA10) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART1 Tx (PA9) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);

    /* Configure the USARTx */
    USART_Init(USART1, &USART_InitStructure);

    NVIC_EnableIRQ(USART1_IRQn);

    /* Enable the USARTx */
    USART_Cmd(USART1, ENABLE);
}
//---------------------------------------------------------------------------------------------
void serial_put_char(int port, char txchar, TickType_t block_ticks)
{
    if (xQueueSend(txchars_queue, &txchar, block_ticks) == pdPASS)
    {
        USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    }
}
int fputc(int ch, FILE * f)
{
    serial_put_char(0, (uint8_t) ch, 0);
    return ch;
}
//---------------------------------------------------------------------------------------------
static inline void str_echo(char *str, int len)
{
    while (len--)
    {
        xQueueSendFromISR(txchars_queue, str++, 0);
    }
    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

static void cmd_echo(char ch)
{
    if ((ch == 0x08 || ch == 0x7F) && cmd_args.len > 0)//退格或删除
    {
        char str[] = {0x08, ' ', 0x08};
        cmd_args.len--;
        str_echo(str, sizeof(str));
    }
    else
    {
        if (ch > 0x1f && ch < 0x80)
        {
            if (cmd_args.len < sizeof(cmd_args.cmdline) - 1)
            {
                cmd_args.cmdline[cmd_args.len++] = ch;
                str_echo(&ch, 1);
            }
        }
        else if (ch == 0x0D || ch == 0x0A)//newline carriage return
        {
            char str[] = {0x0D, 0x0A};

            cmd_args.cmdline[cmd_args.len++] = '\0';
            str_echo(str, sizeof(str));

            xSemaphoreGiveFromISR(xRxCompleteSemaphore, NULL);
        }
    }
}
//---------------------------------------------------------------------------------------------
void debug_port_interrupt_handler(void)
{
    char c;

    if (USART_GetITStatus(USART1, USART_IT_TXE) == SET)
    {
        if (xQueueReceiveFromISR(txchars_queue, &c, NULL) == pdPASS)
        {
            USART_SendData(USART1, c);
        }
        else
        {
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }
    }

    if (USART_GetITStatus(USART1, USART_IT_RXNE))
    {
        c = USART_ReceiveData(USART1);
        cmd_echo(c);
    }
}
//---------------------------------------------------------------------------------------------
static int make_argv(char *cmdline, char *argv[])
{
    int argc = 0, i = 0, in_text = 0;

    while (cmdline[i])
    {
        if (((cmdline[i] == ' ') || (cmdline[i] == '\t')))
        {
            if (in_text)
            {
                cmdline[i] = '\0';
                in_text = 0;
            }
        }
        else
        {
            if (!in_text)
            {
                in_text = 1;
                if (argc >= UIF_MAX_ARGS) break;
                if (argv) argv[argc] = &cmdline[i];
                argc++;
            }
        }
        i++;
    }
    if (argv) argv[argc] = NULL;
    return argc;
}

//---------------------------------------------------------------------------------------------
static int uif_cmd_help(int argc, char **argv);
static int get_task_state(int argc, char **argv);
static int handle_reset(int argc, char **argv);

uif_cmd_t uif_cmdtab[] = 
{
    {"help", 1, 0, uif_cmd_help, "Help", "<cmd>"},
    {"task", 1, 0, get_task_state, "task", "<cmd>"},
    {"reset", 0, 0, handle_reset, "reset", "<cmd>"},
};

#define UIF_NUM_CMD array_size(uif_cmdtab)
//---------------------------------------------------------------------------------------------
static const char HELPFORMAT[] = "%s\t";
static void help_display(int index)
{
    printf(HELPFORMAT, uif_cmdtab[index].cmd);
    if(index+1 >= UIF_NUM_CMD ) printf("\r\n");
}

static int uif_cmd_help(int argc, char **argv)
{
    //printf(HELPFORMAT, "Command", "help", "usage", "");
    for (int i = 0; i < UIF_NUM_CMD; i++)
    {
        help_display(i);
        vTaskDelay(100/portTICK_RATE_MS);
    }
    return 0;
}

static int get_task_state(int argc, char **argv)
{
    TaskStatus_t *pxTaskStatusArray;
	volatile UBaseType_t uxArraySize, x;
	uint32_t ulTotalRunTime, ulStatsAsPercentage;

	uxArraySize = uxTaskGetNumberOfTasks();
    
	pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );
	if( pxTaskStatusArray != NULL )
	{
        printf("任务名\t\tID\t优先级\t堆栈\tCPU使用率\r\n");
		uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalRunTime );
        if( ulTotalRunTime > 0 )
	    {
            for( x = 0; x < uxArraySize; x++ )
    		{
                ulStatsAsPercentage = pxTaskStatusArray[ x ].ulRunTimeCounter / ulTotalRunTime * 100;
                if( ulStatsAsPercentage > 0UL )
				{
        			printf("%-16s%-8d%-8d%-8d%d%%\r\n", pxTaskStatusArray[x].pcTaskName,pxTaskStatusArray[x].xTaskNumber,
                        pxTaskStatusArray[x].uxCurrentPriority, pxTaskStatusArray[x].usStackHighWaterMark, ulStatsAsPercentage);
                    vTaskDelay(100/portTICK_RATE_MS);
                }
                else
                {
                    printf("%-16s%-8d%-8d%-8d<1%%\r\n", pxTaskStatusArray[x].pcTaskName,pxTaskStatusArray[x].xTaskNumber,
                        pxTaskStatusArray[x].uxCurrentPriority, pxTaskStatusArray[x].usStackHighWaterMark);
                    vTaskDelay(100/portTICK_RATE_MS);
                }
    		}
    		
        }
        vPortFree( pxTaskStatusArray );
	}
    printf("\r\n");
    size_t freeHeapSize = xPortGetFreeHeapSize();
    printf("the free heapsize is %d\r\n", freeHeapSize);  
    vTaskDelay(50/portTICK_RATE_MS);
    return 0;
}

static int handle_reset(int argc, char **argv)
{
    __set_FAULTMASK(1);
    NVIC_SystemReset();
    return 0;
}
//---------------------------------------------------------------------------------------------
static void run_cmd(void)
{
    int i, argc;
    char cmd_tmp[UIF_MAX_LINE];
    char *argv[UIF_MAX_ARGS + 1];

    cmd_args.len = 0;
    strcpy(cmd_tmp, cmd_args.cmdline);

    if (!(argc = make_argv(cmd_tmp, argv)))
        goto out;

    for (i = 0; i < array_size(uif_cmdtab); ++i)
    {
        if (!strcasecmp(uif_cmdtab[i].cmd, argv[0]))
        {
            if (argc - 1 > uif_cmdtab[i].max_args)
            {
                printf("Usage: %s\n\r", uif_cmdtab[i].usage);
                goto out;
            }
            uif_cmdtab[i].func(argc, argv);
            goto out;
        }
    }
    printf("Error: No such command: %s\n\r", argv[0]);
out:
    printf(PROMPT);
}
//---------------------------------------------------------------------------------------------
void debug_port_task_handler(void *pvParameters)
{
    setup_debug_port_peri();

    for (;;)
    {
        if (xSemaphoreTake(xRxCompleteSemaphore, portMAX_DELAY) == pdPASS) 
            run_cmd();
    }
}
//---------------------------------------------------------------------------------------------

