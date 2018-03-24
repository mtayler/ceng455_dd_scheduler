#ifndef __periodic_task_H
#define __periodic_task_H

#include <mqx.h>

#include "fsl_device_registers.h"
#include "clockMan1.h"
#include "pin_init.h"
#include "osa1.h"
#include "mqx_ksdk.h"
#include "uart1.h"
#include "fsl_hwtimer1.h"
#include "MainTask.h"
#include "Generator.h"
#include "Scheduler.h"
#include "Monitor.h"
#include "PeriodicTask.h"
#include "GPIO_RGB.h"
#include "HF1.h"
#include "Cpu.h"

#define PERIODIC_BASE (10)
#define INIT_PRIORITY (24)

#define PERIODIC_TASKS (3)

struct periodic_task {
	uint32_t period;
	uint32_t execution_time;
	enum _GPIO_RGB_pinNames pin;
};

struct periodic_task periodic_tasks[PERIODIC_TASKS] = {
	  //   1          2          3
		{1500, 350, LEDRGB_RED}, {500, 100, LEDRGB_GREEN}, {2000, 600, LEDRGB_BLUE},
};

void PeriodicTask_task(os_task_param_t task_init_data);

#endif 
