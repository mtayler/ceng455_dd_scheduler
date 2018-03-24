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

#define UPDATE_PERIOD 2000

#define TICKS_TO_DOUBLE(ticks) ( \
		((double)((uint64_t)*(ticks.TICKS))*_time_get_hwticks_per_tick() ) \
			+ ticks.HW_TICKS)

static MQX_TICK_STRUCT counter;

static void update_monitor(_timer_id timer, void * data,
		MQX_TICK_STRUCT_PTR monitor_start_time) {
	static MQX_TICK_STRUCT monitor_end_time;

	// Being preempted skews timing stats
	_task_stop_preemption();

	MQX_TICK_STRUCT monitor_run_time;
	MQX_TICK_STRUCT system_run_time;

	// Calculate system running time
	_time_diff_ticks(monitor_start_time, &monitor_end_time, &system_run_time);

	double system_time = TICKS_TO_DOUBLE(system_run_time);
	double monitor_time = TICKS_TO_DOUBLE(counter);
	_time_init_ticks(&counter, 0);
	double total_time = system_time+monitor_time;

	printf("\nProcessor utilization: %.4f%%", 100*system_time/(system_time+monitor_time));

	_time_get_elapsed_ticks(&monitor_end_time);
	_task_start_preemption();

	// Calculate our running time and overhead (try to be quick)
	_time_diff_ticks(&monitor_end_time, monitor_start_time, &monitor_run_time);
	double update_time = TICKS_TO_DOUBLE(monitor_run_time);
	printf(" Monitor overhead: %.4f%%\n\n", update_time/total_time);
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
	_time_init_ticks(timer_period, 0);
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
	_time_init_ticks(&counter, 0);
	MQX_TICK_STRUCT last_update;
	MQX_TICK_STRUCT now;
	bool overflow = FALSE;
	while (1) {
		_int_disable();
		_time_get_elapsed_ticks(&now);
		int32_t diff = _time_diff_microseconds(&now, &last_update, &overflow);
		_time_add_usec_to_ticks(&counter, diff);	// assume no overflow
		_time_get_elapsed_ticks(&last_update);
		_int_enable();
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
