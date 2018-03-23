#ifndef __periodic_task_H
#define __periodic_task_H

#include <mqx.h>

#include "Cpu.h"

#define PERIODIC_BASE (10)
#define INIT_PRIORITY (24)

struct periodic_task {
	time_t period;
	time_t execution_time;
};

struct periodic_task periodic_tasks[] = {
	  //   1          2          3
		{125, 50}, {150, 25}, {800u, 200},
};

void PeriodicTask_task(os_task_param_t task_init_data);

#endif 
