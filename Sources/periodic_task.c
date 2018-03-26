#include "Cpu.h"
#include "Events.h"
#include "rtos_main_task.h"
#include "generator_task.h"
#include "scheduler_task.h"
#include "monitor_task.h"
#include "periodic_task.h"
#include "os_tasks.h"

#include "dd_scheduler.h"

void PeriodicTask_task(os_task_param_t task_init_data)
{
	GPIO_DRV_ClearPinOutput(periodic_tasks[task_init_data].pin);
	for (uint32_t i=0; i < DELAY_CONST*periodic_tasks[task_init_data].execution_time; i++);
	GPIO_DRV_SetPinOutput(periodic_tasks[task_init_data].pin);

	uint32_t result = dd_delete(_task_get_id());
	assert(result == MQX_OK);
}
