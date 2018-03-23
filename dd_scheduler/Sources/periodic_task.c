#include "Cpu.h"
#include "Events.h"
#include "rtos_main_task.h"
#include "generator_task.h"
#include "scheduler_task.h"
#include "monitor_task.h"
#include "periodic_task.h"

#include "dd_scheduler.h"

void PeriodicTask_task(os_task_param_t task_init_data)
{
	GPIO_DRV_WritePinOutput(periodic_tasks[task_init_data].pin, 0);
	for (int i=0; i < 1000*periodic_tasks[task_init_data].execution_time; i++);
	GPIO_DRV_WritePinOutput(periodic_tasks[task_init_data].pin, 1);

	dd_delete(_task_get_id());
}
