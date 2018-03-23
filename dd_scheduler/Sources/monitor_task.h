/* ###################################################################
**     Filename    : monitor_task.h
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
** @file monitor_task.h
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup monitor_task_module monitor_task module documentation
**  @{
*/         

#ifndef __monitor_task_H
#define __monitor_task_H
/* MODULE monitor_task */

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

#ifdef __cplusplus
extern "C" {
#endif 

/*
** ===================================================================
**     Callback    : Monitor_task
**     Description : Task function entry.
**     Parameters  :
**       task_init_data - OS task parameter
**     Returns : Nothing
** ===================================================================
*/
void Monitor_task(os_task_param_t task_init_data);



/* END monitor_task */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

#endif 
/* ifndef __monitor_task_H*/
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
