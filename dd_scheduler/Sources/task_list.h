/*
 * task_list.h
 *
 *  Created on: Mar 22, 2018
 *      Author: mtayler
 */

#ifndef SOURCES_TASK_LIST_H_
#define SOURCES_TASK_LIST_H_

#include <mqx.h>
#include <timer.h>

// List of running tasks
typedef struct task_list {
	uint32_t tid;
	TIME_STRUCT deadline;
	uint32_t task_type;
	TIME_STRUCT creation_time;
	_timer_id timer;
	struct task_list *next_cell;
	struct task_list *previous_cell;
} task_list_t, * task_list_ptr;

//struct task_list * add_new_task(struct task_list ** list, _task_id tid, time_t deadline, uint32_t task_type, time_t creation_time, _timer_id timer);
task_list_ptr add_task(task_list_t ** list, task_list_ptr task);
_mqx_uint delete_task(task_list_ptr * list, _task_id tid);
task_list_ptr get_task(task_list_ptr list, _task_id tid);
_mqx_uint sort_tasks(task_list_ptr list);
_mqx_uint assign_priorities(task_list_ptr list);

#endif /* SOURCES_TASK_LIST_H_ */
