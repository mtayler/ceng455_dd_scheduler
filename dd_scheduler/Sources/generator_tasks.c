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
#include "generator_tasks.h"
#include "scheduler_task.h"
#include "monitor_task.h"

#ifdef __cplusplus
extern "C" {
#endif 


/* User includes (#include below this line is not maintained by Processor Expert) */
#include <mqx.h>

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
  /* Write your local variable definition here */

#define PERIODIC_BASE (100)

	struct release_times {
		struct periodic_task * task;
		time_t last_release;
	} release_times;
  
#ifdef PEX_USE_RTOS
  while (1) {
#endif
	  /* Write your code here ... */
	  // for each release_times entry
	  //     if time since last release > period
	  //          dd_tcreate(...)
	  //          release_times.last_release = now

    OSA_TimeDelay(10);                 /* Example code (for task release) */
    
#ifdef PEX_USE_RTOS   
  }
#endif    
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
