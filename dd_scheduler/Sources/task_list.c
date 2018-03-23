/*
 * task_list.c
 *
 *  Created on: Mar 22, 2018
 *      Author: mtayler
 */

#include "task_list.h"

//task_list_ptr create_task(task_list_ptr* list, _task_id tid,
//		time_t deadline, uint32_t task_type, time_t creation_time, _timer_id timer) {
//
//	task_list_ptr task = _mem_alloc(sizeof(task_list_t));
//	task->tid = tid;
//	task->deadline = deadline;
//	task->task_type = task_type;
//	task->creation_time = creation_time;
//	task->timer = timer;
//	task->next_cell = NULL;
//	task->previous_cell = NULL;
//
//	return add_task(list, task);
//}

task_list_ptr add_task(task_list_ptr * list, task_list_ptr task) {

	if (list == NULL) {
		*list = task;
		return MQX_OK;
	}

	// If new first element
	if (task->deadline.SECONDS < (*list)->deadline.SECONDS) {
		task->next_cell = *list;
		(*list)->previous_cell = task;
		*list = task;
		return task;
	}

	task = *list;
	while (task->next_cell != NULL &&
			task->deadline.SECONDS > task->next_cell->deadline.SECONDS) {
		task = task->next_cell;
	}
	task->next_cell = task->next_cell;
	task->previous_cell = task;
	task->next_cell = task;

	return task;
}

task_list_ptr get_task(task_list_ptr list, _task_id tid) {
	task_list_ptr task = list;
	while (task->tid != tid && task->next_cell != NULL) {
		task = task->next_cell;
	}
	if (task->tid != tid) {
		_task_set_error(MQX_INVALID_PARAMETER);
		return NULL;
	} else {
		return task;
	}
}

_mqx_uint delete_task(task_list_ptr * list, _task_id tid) {
	task_list_ptr task = *list;
	while (task->tid != tid && task->next_cell != NULL) {
		task = task->next_cell;
	}
	if (task->tid != tid) {
		return MQX_INVALID_PARAMETER;
	}

	task_list_ptr p = task->previous_cell;
	if (p != NULL) {
		p->next_cell = task->next_cell;
	}
	task->next_cell->previous_cell = p;
}

_mqx_uint sort_tasks(task_list_ptr list);

_mqx_uint assign_priorities(task_list_ptr list) {
	if (list == NULL) {
		return MQX_INVALID_POINTER;
	}

	_mqx_uint priority = 1;	// highest priority below scheduler task
	while (list->next_cell != NULL) {
		_task_set_priority(list->tid, priority, NULL);
		// increase priority until min priority
		if (priority < _sched_get_min_priority(MQX_SCHED_FIFO)) {
			priority++;
		}
		list = list->next_cell;
	}
	return MQX_OK;
}
