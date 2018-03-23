/* ###################################################################
**     Filename    : scheduler_task.c
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
**         Scheduler_task - void Scheduler_task(os_task_param_t task_init_data);
**
** ###################################################################*/
/*!
** @file scheduler_task.c
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup scheduler_task_module scheduler_task module documentation
**  @{
*/         
/* MODULE scheduler_task */

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
#include "dd_scheduler.h"

#define INIT_MESSAGES (4)
#define GROW_MESSAGES (2)

/*
** ===================================================================
**     Callback    : Scheduler_task
**     Description : Task function entry.
**     Parameters  :
**       task_init_data - OS task parameter
**     Returns : Nothing
** ===================================================================
*/
void Scheduler_task(os_task_param_t task_init_data)
{
	// Create the message pool with room for the largest message
	_pool_id scheduler_message_pool = _msgpool_create(
			sizeof(SCHEDULER_RQST_MSG), INIT_MESSAGES, GROW_MESSAGES, 0);
	_pool_id scheduler_resp_pool = _msgpool_create(
			sizeof(SCHEDULER_RESP_MSG), INIT_MESSAGES, GROW_MESSAGES, 0);

	_time_delay(5); // Allow other tasks to initialize
  
#ifdef PEX_USE_RTOS
	while (1) {
#endif
		/* Write your code here ... */

		// Handler for scheduler requests go here.
		// Receive dd_scheduler request messages and handle request,
		// then reply.

		for (_mqx_uint i=0; i < 100000000000; i++);

		_time_delay(1);


#ifdef PEX_USE_RTOS   
	}
#endif    
}

/* END scheduler_task */

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
