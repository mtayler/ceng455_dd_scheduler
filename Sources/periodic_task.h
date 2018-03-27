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
#include "gpio1.h"
#include "HF1.h"
#include "Aperiodic.h"
#include "Cpu.h"

#define PERIODIC_BASE (10)
#define INIT_PRIORITY (24)

#define PERIODIC_TASKS (3)

#define DELAY_CONST 14500

struct periodic_task {
	uint32_t period;
	uint32_t execution_time;
	uint32_t deadline;
	enum _gpio1_pinNames pin;
};

struct periodic_task periodic_tasks[PERIODIC_TASKS] = {
//		 PERIOD     EXEC    DL      LED
//		{1000,      100,    1000,   LEDRGB_RED},
//		{500,       300,    500,    LEDRGB_GREEN},
//		{250,       100,    250,    LEDRGB_BLUE},
		{1000,      300,    800,    LEDRGB_RED},
		{1000,      350,    1000,   LEDRGB_GREEN},
		{300,       50,    	300,    LEDRGB_BLUE}
};

void PeriodicTask_task(os_task_param_t task_init_data);

#endif 
