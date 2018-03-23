/*
 * task_list.c
 *
 *  Created on: Mar 22, 2018
 *      Author: mtayler
 */

#include "task_list.h"

#include <stdio.h>

task_list_ptr add_task(task_list_ptr * list, task_list_ptr task) {
	if (*list == NULL) {
		*list = task;
		return MQX_OK;
	}

	task->next_cell = NULL;
	task->previous_cell = NULL;

	// If new first element
	if (task->deadline.MILLISECONDS < (*list)->deadline.MILLISECONDS) {
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
	if (*list == NULL) {
		printf("delete_task: no tasks in list");
		return MQX_INVALID_POINTER;
	}

	task_list_ptr task = *list;

	while (task->tid != tid && task->next_cell != NULL) {
		task = task->next_cell;
	}
	if (task->tid != tid) {
		return MQX_INVALID_PARAMETER;
	}

	if (task == *list) {
		*list = task->next_cell;
	} else {
		task_list_ptr p = task->previous_cell;
		if (p != NULL) {
			p->next_cell = task->next_cell;
		}
		if (task->next_cell != NULL) {
			task->next_cell->previous_cell = p;
		}
	}

	return MQX_OK;
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
