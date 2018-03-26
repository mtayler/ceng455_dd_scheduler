/* ###################################################################
**     Filename    : generator_tasks.h
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
** @file generator_tasks.h
** @version 01.00
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup generator_tasks_module generator_tasks module documentation
**  @{
*/         

#ifndef __generator_tasks_H
#define __generator_tasks_H
/* MODULE generator_tasks */

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
#include "gpio1.h"
#include "HF1.h"
#include "Aperiodic.h"

#include <message.h>

#ifdef __cplusplus
extern "C" {
#endif 

_queue_id generator_msg_qid;

typedef struct generator_msg {
	MESSAGE_HEADER_STRUCT HEADER;
	uint32_t task_template;
	uint32_t parameter;
	uint32_t deadline;
} GENERATOR_MSG, * GENERATOR_MSG_PTR;

/*
** ===================================================================
**     Callback    : Generator_task
**     Description : Task function entry.
**     Parameters  :
**       task_init_data - OS task parameter
**     Returns : Nothing
** ===================================================================
*/
void Generator_task(os_task_param_t task_init_data);

/* END generator_tasks */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

#endif 
/* ifndef __generator_tasks_H*/
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
