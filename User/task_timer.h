#ifndef _TASK_TIMER_H_
#define _TASK_TIMER_H_

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define TIMER_NR 3

struct TaskTimer
{
    TimerHandle_t timer;
    int period;
    TimerCallbackFunction_t handler;
};

extern struct TaskTimer timers[TIMER_NR];

void setup_timers(void);

#endif
