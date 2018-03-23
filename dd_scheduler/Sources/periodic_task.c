#include "periodic_task.h"

void PeriodicTask_task(os_task_param_t task_init_data)
{
	for (int i=0; i < 1000*periodic_tasks[task_init_data].execution_time; i++);
}
