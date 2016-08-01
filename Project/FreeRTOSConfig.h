#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION		1   //配置为1使用抢占式内核,配置为0使用时间片 
#define configUSE_TICKLESS_IDLE     0   //1使能低功耗tickless模式，为0保持系统节拍（tick）中断一直运行
#define configCPU_CLOCK_HZ			( ( unsigned long ) 72000000 )
#define configTICK_RATE_HZ			( ( TickType_t ) 100 )
#define configMAX_PRIORITIES		( 5 )
#define configMINIMAL_STACK_SIZE	( ( unsigned short ) 128 )  //定义空闲任务使用的堆栈大小
#define configTOTAL_HEAP_SIZE		( ( size_t ) ( 17 * 1024 ) )    //内核总共可用RAM 
#define configMAX_TASK_NAME_LEN		( 16 )
#define configUSE_TRACE_FACILITY	1   //表示启动可视化跟踪调试，会激活一些附加的结构体成员和函数
#define configUSE_16_BIT_TICKS		0   //定义系统节拍计数器的变量类型
#define configIDLE_SHOULD_YIELD		1
#define configUSE_MUTEXES           0
#define configUSE_RECURSIVE_MUTEXES      0  //使用递归互斥量
#define configUSE_COUNTING_SEMAPHORES    0  //使用计数信号量
#define configUSE_QUEUE_SETS             1  //使能队列集功能


#define configUSE_TIMERS            1   //使用软件定时器
#define configTIMER_TASK_PRIORITY   1   //软件定时器服务/守护进程的优先级
#define configTIMER_QUEUE_LENGTH    10
#define configTIMER_TASK_STACK_DEPTH 128
#define configGENERATE_RUN_TIME_STATS 1 //使能运行时间统计功能

/*Hook function related definitions. */ 
#define configUSE_IDLE_HOOK			    0   //void vApplicationIdleHook( void ) 
#define configUSE_TICK_HOOK			    0   //void vApplicationTickHook( void );
#define configCHECK_FOR_STACK_OVERFLOW  0
#define configUSE_MALLOC_FAILED_HOOK    0   //void vApplicationMallocFailedHook( void);

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() ( ulHighFrequencyTimerTicks = 0UL)  
#define portGET_RUN_TIME_COUNTER_VALUE() ulHighFrequencyTimerTicks

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 		0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet		1
#define INCLUDE_uxTaskPriorityGet		1
#define INCLUDE_vTaskDelete				1
#define INCLUDE_vTaskCleanUpResources	0
#define INCLUDE_vTaskSuspend			1
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay				1

/* This is the raw value as per the Cortex-M3 NVIC.  Values can be 255
(lowest) to 0 (1?) (highest). */
#define configKERNEL_INTERRUPT_PRIORITY 		255 //用来设置RTOS内核自己的中断优先级
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	191 /* equivalent to 0xb0, or priority 11. */


/* This is the value being used as per the ST library which permits 16
priority values, 0 to 15.  This must correspond to the
configKERNEL_INTERRUPT_PRIORITY setting.  Here 15 corresponds to the lowest
NVIC value of 255. */
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY	15

#endif                          /* FREERTOS_CONFIG_H */
