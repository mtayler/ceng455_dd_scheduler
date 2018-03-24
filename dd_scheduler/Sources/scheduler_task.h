/* ###################################################################
**     Filename    : scheduler_task.h
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
** @file scheduler_task.h
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup scheduler_task_module scheduler_task module documentation
**  @{
*/         

#ifndef __scheduler_task_H
#define __scheduler_task_H
/* MODULE scheduler_task */

#include "fsl_device_registers.h"
#include "clockMan1.h"
#include "pin_init.h"
#include "osa1.h"
#include "mqx_ksdk.h"
#include "uart1.h"
#include "fsl_hwtimer1.h"
#include "MainTask.h"
#include "Generator.h"
#include "Scheduler.h"
#include "Monitor.h"
#include "PeriodicTask.h"
#include "GPIO_RGB.h"
#include "HF1.h"

#ifdef __cplusplus
extern "C" {
#endif 

/*
** ===================================================================
**     Callback    : Scheduler_task
**     Description : Task function entry.
**     Parameters  :
**       task_init_data - OS task parameter
**     Returns : Nothing
** ===================================================================
*/
void Scheduler_task(os_task_param_t task_init_data);

/* END scheduler_task */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

#endif 
/* ifndef __scheduler_task_H*/
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
