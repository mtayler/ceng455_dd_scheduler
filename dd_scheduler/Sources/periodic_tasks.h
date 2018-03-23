/*
 * periodic_tasks.h
 *
 *  Created on: Mar 20, 2018
 *      Author: mtayler
 */

#ifndef SOURCES_PERIODIC_TASKS_H_
#define SOURCES_PERIODIC_TASKS_H_

#include <mqx.h>

#define PERIODIC_BASE (10)
#define INIT_PRIORITY (24)

struct periodic_task {
	uint8_t id;
	time_t period;
	time_t execution_time;
};

struct periodic_task p_task1 = {PERIODIC_BASE+1, 125, 50};
struct periodic_task p_task2 = {PERIODIC_BASE+2, 150, 25};
struct periodic_task p_task3 = {PERIODIC_BASE+3, 800, 200};

void p_task1_task() {
	uint32_t i=0;
	while (i < p_task1->execution_time*1000);
}

void p_task2_task() {
	uint32_t i=0;
	while (i < p_task2->execution_time*1000);
}

void p_task3_task() {
	uint32_t i=0;
	while (i < p_task3->execution_time*1000);
}

TASK_TEMPLATE_STRUCT task_templates[] = {
	{0, p_task1_task, 10, INIT_PRIORITY
	  "PeriodicTask1", NULL, 0, 0},
//	{0, p_task2_task, 10, INIT_PRIORITY,
//			"PeriodicTask2" NULL, 0, 0}
};

#endif /* SOURCES_PERIODIC_TASKS_H_ */
