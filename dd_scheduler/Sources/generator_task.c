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
	MQX_TICK_STRUCT current_ticks;
	TIME_STRUCT current_time;
	MQX_TICK_STRUCT release_times[PERIODIC_TASKS];
	bool overflow = FALSE;

	while (1) {
		_time_get_elapsed_ticks(&current_ticks);
		for (uint8_t i=0; i < PERIODIC_TASKS; i++) {

			int32_t elapsed = _time_diff_milliseconds(
					&current_ticks, &release_times[i], &overflow);

			if (overflow | (elapsed > periodic_tasks[i].period)) {
				_time_get_elapsed_ticks(&release_times[i]);

				_ticks_to_time(&current_ticks, &current_time);
				dd_tcreate(i, periodic_tasks[i].period);

			}
		}
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
