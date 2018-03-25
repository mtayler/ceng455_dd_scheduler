#include "Cpu.h"
#include "Events.h"
#include "rtos_main_task.h"
#include "generator_task.h"
#include "scheduler_task.h"
#include "monitor_task.h"
#include "periodic_task.h"
#include "os_tasks.h"

#include "dd_scheduler.h"

#define DELAY_CONST 5000

void PeriodicTask_task(os_task_param_t task_init_data)
{
	GPIO_DRV_WritePinOutput(periodic_tasks[task_init_data].pin, 0);
	for (uint32_t i=0; i < DELAY_CONST*periodic_tasks[task_init_data].execution_time; i++);
	GPIO_DRV_WritePinOutput(periodic_tasks[task_init_data].pin, 1);

	dd_delete(_task_get_id());
}
