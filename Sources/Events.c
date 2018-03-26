/* ###################################################################
**     Filename    : Events.c
**     Project     : dd_scheduler
**     Processor   : MK22FN512VLH12
**     Component   : Events
**     Version     : Driver 01.00
**     Compiler    : GNU C Compiler
**     Date/Time   : 2018-03-17, 12:10, # CodeGen: 0
**     Abstract    :
**         This is user's event module.
**         Put your event handler code here.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file Events.c
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup Events_module Events module documentation
**  @{
*/         
/* MODULE Events */

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
#include <stdio.h>
#include "dd_scheduler.h"

/*
** ===================================================================
**     Interrupt handler : gpio1_PORTB_IRQHandler
**
**     Description :
**         User interrupt service routine. 
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void gpio1_PORTB_IRQHandler(void)
{
  /* Clear interrupt flag.*/
  PORT_HAL_ClearPortIntFlag(PORTB_BASE_PTR);
  /* Write your code here ... */
  GENERATOR_MSG_PTR msg = _msg_alloc(scheduler_msg_pool);
  assert(msg != NULL);
  msg->HEADER.TARGET_QID = generator_msg_qid;
  msg->HEADER.SOURCE_QID = MSGQ_NULL_QUEUE_ID;
  msg->HEADER.SIZE = sizeof(GENERATOR_MSG);
  msg->task_template = APERIODIC_TASK;
  msg->parameter = 0;
  msg->deadline = 600;

  _msgq_send(msg);
}

/* END Events */

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
