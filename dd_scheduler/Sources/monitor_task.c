/* ###################################################################
**     Filename    : monitor_task.c
**     Project     : dd_scheduler
**     Processor   : MK22FN512VLH12
**     Component   : Events
**     Version     : Driver 01.00
**     Compiler    : GNU C Compiler
**     Date/Time   : 2018-03-20, 19:27, # CodeGen: 7
**     Abstract    :
**         This is user's event module.
**         Put your event handler code here.
**     Settings    :
**     Contents    :
**         Monitor_task - void Monitor_task(os_task_param_t task_init_data);
**
** ###################################################################*/
/*!
** @file monitor_task.c
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup monitor_task_module monitor_task module documentation
**  @{
*/         
/* MODULE monitor_task */

#include "Cpu.h"
#include "Events.h"
#include "rtos_main_task.h"
#include "generator_task.h"
#include "scheduler_task.h"
#include "monitor_task.h"
#include "periodic_task.h"

#ifdef __cplusplus
extern "C" {
#endif 

/* User includes (#include below this line is not maintained by Processor Expert) */
#include <mqx.h>
#include <stdio.h>

#include "dd_scheduler.h"
#include "task_list.h"

static uint64_t get_hwticks_plus_ticks(MQX_TICK_STRUCT_PTR t);

#define UPDATE_PERIOD 1000

static void print_tasks(task_list_ptr task) {
	while (task != NULL) {
		TASK_TEMPLATE_STRUCT_PTR task_template = _task_get_template_ptr(task->tid);
		char * name = "DONE";
		if (task_template != NULL) {	// if the task hasn't finished yet
			name = task_template->TASK_NAME;
		}
		printf("\t%-20s %8lu %4lu.%04lus %4lu.%04lus\n",
				name, task->tid,
				task->deadline.SECONDS, task->deadline.MILLISECONDS,
				task->creation_time.SECONDS, task->creation_time.MILLISECONDS);
		task = task->next_cell;
	}
}


/*
** ===================================================================
**     Callback    : Monitor_task
**     Description : Task function entry.
**     Parameters  :
**       task_init_data - OS task parameter
**     Returns : Nothing
** ===================================================================
*/
void Monitor_task(os_task_param_t task_init_data)
{
	MQX_TICK_STRUCT last_update_time;
	bool overflow;

	double system_time;
	double monitor_time;

	MQX_TICK_STRUCT monitor_start_time;
	MQX_TICK_STRUCT monitor_end_time;
	MQX_TICK_STRUCT monitor_run_time;
	MQX_TICK_STRUCT system_run_time;

	_time_init_ticks(&last_update_time, UINT32_MAX);

	_time_init_ticks(&monitor_start_time, 0);
	_time_init_ticks(&monitor_end_time, 0);
	_time_init_ticks(&monitor_run_time, UINT32_MAX);
	_time_init_ticks(&system_run_time, 0);

	printf("\x1b[2J");
  
#ifdef PEX_USE_RTOS
	while (1) {
#endif
		// Lock before disabling preemption to avoid deadlock
		_mutex_lock(&active_mutex);
		_mutex_lock(&overdue_mutex);

		// Being preempted skews timing stats
		_task_stop_preemption();

		// Rate limit updates
		_time_get_elapsed_ticks(&monitor_start_time);
		if ((_time_diff_milliseconds(&monitor_start_time,
				&last_update_time, &overflow) > UPDATE_PERIOD) | overflow) {

			_time_get_elapsed_ticks(&last_update_time);

			// Calculate system running time
			_time_diff_ticks(&monitor_start_time, &monitor_end_time,
					&system_run_time);

			system_time = get_hwticks_plus_ticks(&system_run_time);

			// Print list of tasks
			task_list_ptr task;
			dd_active_list(&task);
			printf("\nACTIVE TASKS:\n\t%-20s %8s %10s %10s\n",
					"NAME", "ID", "DEADLINE", "CREATION");
			print_tasks(task);

			dd_overdue_list(&task);
			printf("\nOVERDUE TASKS:\n\t%-20s %8s %10s %10s\n",
					"NAME", "ID", "DEADLINE", "CREATION");
			print_tasks(task);

			// Calculate our running time
			_time_get_elapsed_ticks(&monitor_end_time);
			_time_diff_ticks(&monitor_end_time, &monitor_start_time,
					&monitor_run_time);

			monitor_time = get_hwticks_plus_ticks(&monitor_run_time);

			printf("\nProcessor utilization: %.4f%%\n\n", 100*system_time/(system_time+monitor_time));
		}
		_task_start_preemption();

		_mutex_unlock(&overdue_mutex);
		_mutex_unlock(&active_mutex);

		_sched_yield();
#ifdef PEX_USE_RTOS   
	}
#endif    
}

uint64_t get_hwticks_plus_ticks(MQX_TICK_STRUCT_PTR t) {
	return (uint64_t)t->HW_TICKS + (_time_get_hwticks_per_tick()*(
			(uint64_t)t->TICKS[1] << 32 | t->TICKS[0]));
}

/* END monitor_task */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
