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
#include "generator_tasks.h"
#include "scheduler_task.h"
#include "monitor_task.h"
#include "periodic_task.h"

#ifdef __cplusplus
extern "C" {
#endif 

/* User includes (#include below this line is not maintained by Processor Expert) */
#include <mqx.h>
#include <stdio.h>

static uint64_t get_hwticks_plus_ticks(MQX_TICK_STRUCT_PTR t);

#define PRINT_PERIOD 1000
#define MONITOR_DELAY 100


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
	MQX_TICK_STRUCT last_print_time;
	bool print_overflow;

	_mqx_uint utilization;
	uint64_t system_time;
	uint64_t monitor_time;

	MQX_TICK_STRUCT monitor_start_time;
	MQX_TICK_STRUCT monitor_end_time;
	MQX_TICK_STRUCT monitor_run_time;
	MQX_TICK_STRUCT system_run_time;

	_time_init_ticks(&last_print_time, ((_mqx_uint)0)-1);

	_time_init_ticks(&monitor_start_time, 0);
	_time_init_ticks(&monitor_end_time, 0);
	_time_init_ticks(&monitor_run_time, ((_mqx_uint)0)-1);
	_time_init_ticks(&system_run_time, 0);
  
#ifdef PEX_USE_RTOS
	while (1) {
#endif
		// Ordering is important in an effort to reduce unaccounted for latency

		_task_stop_preemption();	// prevent preemption altering results
		_time_get_elapsed_ticks(&monitor_start_time);
			_time_diff_ticks(&monitor_start_time, &monitor_end_time,
					&system_run_time);

		system_time = get_hwticks_plus_ticks(&system_run_time);

		_time_get_elapsed_ticks(&monitor_end_time);
		_time_diff_ticks(&monitor_end_time, &monitor_start_time,
				&monitor_run_time);

		monitor_time = get_hwticks_plus_ticks(&monitor_run_time);

		_task_start_preemption();

		printf("Processor utilization: %.4f%%\n", 100*system_time/(system_time+monitor_time));

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
