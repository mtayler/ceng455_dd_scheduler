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
#include "os_tasks.h"

#ifdef __cplusplus
extern "C" {
#endif 

/* User includes (#include below this line is not maintained by Processor Expert) */
#include <mqx.h>
#include <stdio.h>

#include "dd_scheduler.h"
#include "task_list.h"
#include "tick_funcs.h"

#define UPDATE_PERIOD 2000
#define MOVING_AVG_COUNT 5


static MQX_TICK_STRUCT counter;
static MQX_TICK_STRUCT overhead_counter;

static float util_samples[MOVING_AVG_COUNT] = {0};
static float overhead_samples[MOVING_AVG_COUNT] = {0};


void monitor_add_overhead_ticks(MQX_TICK_STRUCT_PTR diff) {
	*(overhead_counter.TICKS) += TICKS_VAL(diff->TICKS);
}

static void print_tasks(task_list_ptr active, task_list_ptr overdue) {
	_mqx_uint priority;
	_mqx_uint i=0;
	while (active != NULL) {
		_task_get_priority(active->tid, &priority);
		printf("    %3lu  0x%-5lx   %4lu %8llu.%06lu %8llu.%06lu  %4c\n", priority,
				active->tid, active->task_type, TICKS_VAL(active->deadline.TICKS),
				active->deadline.HW_TICKS, TICKS_VAL(active->creation_time.TICKS),
				active->creation_time.HW_TICKS, i ? 'Y' : 'N');
		active = active->next_cell;
	}
	puts("\n");
	while (overdue != NULL) {
			_task_get_priority(overdue->tid, &priority);
			printf("    %3lu  0x%-5lx   %4lu %8llu.%06lu %8llu.%06lu  %4c\n", priority,
					overdue->tid, overdue->task_type, TICKS_VAL(overdue->deadline.TICKS),
					overdue->deadline.HW_TICKS, TICKS_VAL(overdue->creation_time.TICKS),
					overdue->creation_time.HW_TICKS, 'Y');
			overdue = overdue->next_cell;
		}
}

// Helper function to print the monitor tracked stats
static inline void update_monitor(_timer_id timer, void * data,
		MQX_TICK_STRUCT_PTR timer_time) {
	MQX_TICK_STRUCT monitor_start_time;
	static MQX_TICK_STRUCT monitor_end_time;

	// Get pointers to task lists: most time will be spent in the scheduler.c
	// code which is counted as overhead
	task_list_ptr active_tasks;
	task_list_ptr overdue_tasks;
	_mutex_lock(&tasks_m);
	assert(dd_active_list(&active_tasks) == MQX_OK);
	assert(dd_overdue_list(&overdue_tasks) == MQX_OK);

	// Being preempted skews timing stats
	_task_stop_preemption();
	// We don't want to count time waiting for scheduler to respond with task
	// list (counts towards overhead by scheduler)
	_time_get_elapsed_ticks(&monitor_start_time);

	// Clear screen and home cursor
	puts("\033[2J\033[H");

	printf("CURRENT TIME: %8llu.%lu\n", TICKS_VAL(monitor_start_time.TICKS),
			monitor_start_time.HW_TICKS);
	// Print task lists
	printf("TASKS:\n    %3s  %-7s   %4s %15s %15s  %4s\n",
			"PRI", "TASK ID", "TYPE", "DEADLINE", "CREATION", "OVER");;
	print_tasks(active_tasks, overdue_tasks);
	_mutex_unlock(&tasks_m);


	MQX_TICK_STRUCT monitor_run_time;
	MQX_TICK_STRUCT system_run_time;

	// Calculate system running time
	_time_diff_ticks(&monitor_start_time, &monitor_end_time, &system_run_time);

	float system_time = TICKS_TO_DOUBLE(system_run_time);
	float idle_time = TICKS_TO_DOUBLE(counter);
	counter = _mqx_zero_tick_struct;  // reset counter
	float extern_overhead_time = TICKS_TO_DOUBLE(overhead_counter);
	overhead_counter = _mqx_zero_tick_struct;  // reset counter
	float total_time = system_time+idle_time+extern_overhead_time;

	// update stats
	float util = 0;
	float overhead = 0;
	// shift sample arrays
	for (_mqx_uint i=0; i < MOVING_AVG_COUNT-1; i++) {
		util_samples[i] = util_samples[i+1];
		util += util_samples[i];

		overhead_samples[i] = overhead_samples[i+1];
		overhead += overhead_samples[i];
	}
	// calc utilization
	util_samples[MOVING_AVG_COUNT-1] = 100*(1 - idle_time/total_time);
	util += util_samples[MOVING_AVG_COUNT-1];
	util /= MOVING_AVG_COUNT;

	printf("\nProcessor utilization:\n"
			"\taverage: %2.4f%%\tinstant: %2.4f%%\n", util, util_samples[MOVING_AVG_COUNT-1]);

	_time_get_elapsed_ticks(&monitor_end_time);

	_task_start_preemption();


	// Calculate overhead with update run time (be quick since not timing)
	_time_diff_ticks(&monitor_end_time, &monitor_start_time, &monitor_run_time);

	// Time including
	double total_overhead_time =
			TICKS_TO_DOUBLE(monitor_run_time) + extern_overhead_time;
	// calculate overhead
	overhead_samples[MOVING_AVG_COUNT-1] = total_overhead_time/total_time;
	overhead += overhead_samples[MOVING_AVG_COUNT-1];
	overhead /= MOVING_AVG_COUNT;

	printf("Total overhead:\n"
			"\taverage: %2.4f%%\tinstant: %2.4f%%\n\n\n", overhead, overhead_samples[MOVING_AVG_COUNT-1]);
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
	const MQX_TICK_STRUCT_PTR start = _mem_alloc(sizeof(MQX_TICK_STRUCT));
	const MQX_TICK_STRUCT_PTR timer_period = _mem_alloc(sizeof(MQX_TICK_STRUCT));

	// Setup periodic timer for monitor function
	*timer_period = _mqx_zero_tick_struct;
	_time_add_msec_to_ticks(timer_period, UPDATE_PERIOD);

	_time_get_elapsed_ticks(start);

	_timer_id timer = _timer_start_periodic_at_ticks(update_monitor, NULL,
			TIMER_ELAPSED_TIME_MODE, start, timer_period);

	if (timer == TIMER_NULL_ID) {
		printf("Couldn't start monitor timer");
	}


	// Reset task to minimum priority
	_mqx_uint old_prior;
	_task_set_priority(0, _sched_get_min_priority(MQX_SCHED_FIFO), &old_prior);

	// Track how often we're running
	counter = _mqx_zero_tick_struct;
	MQX_TICK_STRUCT last_update;
	MQX_TICK_STRUCT now;
	bool overflow = FALSE;
	while (1) {
		_task_stop_preemption();
		_time_get_elapsed_ticks(&now);
		int32_t diff = _time_diff_microseconds(&now, &last_update, &overflow);
		_time_add_usec_to_ticks(&counter, diff);	// assume no overflow
		_time_get_elapsed_ticks(&last_update);
		_task_start_preemption();
		_time_delay(1);
	}
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
