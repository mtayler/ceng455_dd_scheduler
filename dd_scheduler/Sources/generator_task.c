/* ###################################################################
**     Filename    : generator_tasks.c
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
**         Generator_task - void Generator_task(os_task_param_t task_init_data);
**
** ###################################################################*/
/*!
** @file generator_tasks.c
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup generator_tasks_module generator_tasks module documentation
**  @{
*/         
/* MODULE generator_tasks */

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
#include <timer.h>

#include "dd_scheduler.h"

/*
** ===================================================================
**     Callback    : Generator_task
**     Description : Task function entry.
**     Parameters  :
**       task_init_data - OS task parameter
**     Returns : Nothing
** ===================================================================
*/
void Generator_task(os_task_param_t task_init_data)
{
  MQX_TICK_STRUCT start_ticks;
  MQX_TICK_STRUCT end_ticks;
  MQX_TICK_STRUCT diff_ticks;
  MQX_TICK_STRUCT elapsed;
  MQX_TICK_STRUCT release_times[PERIODIC_TASKS];

  while (1) {
	_task_stop_preemption();	// don't let scheduler interrupt for timing
	_time_get_elapsed_ticks(&start_ticks);
	for (uint8_t i=0; i < PERIODIC_TASKS; i++) {
		_time_diff_ticks(&start_ticks, &release_times[i], &elapsed);
		if (elapsed.TICKS[0] > periodic_tasks[i].period) {
			_time_get_elapsed_ticks(&release_times[i]);
			dd_tcreate(i, periodic_tasks[i].period);
		}
	}
	_time_get_elapsed_ticks(&end_ticks);
	_time_diff_ticks(&end_ticks, &start_ticks, &diff_ticks);
	monitor_add_overhead_ticks(&diff_ticks);

	_task_start_preemption();	// done with timing
  }
}

/* END generator_tasks */

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
