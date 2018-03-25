/*
 * task_list.c
 *
 *  Created on: Mar 22, 2018
 *      Author: mtayler
 */

#include "task_list.h"

#include "Generator.h"

#include <stdio.h>

_mqx_uint add_task(task_list_ptr * list, task_list_ptr task) {
	task->next_cell = NULL;
	task->previous_cell = NULL;

	// I love sorting less than. So much fun.

	task_list_ptr iter = *list;

	// If list is empty we're the head
	if (iter == NULL) {
		*list = task;
		return MQX_OK;
	}

	// If only one item and we're less than we're the new start
	if (iter->next_cell == NULL &&
			task->deadline.TICKS[1] <= iter->deadline.TICKS[1] &&
			task->deadline.TICKS[0] < iter->deadline.TICKS[0]) {
		task->next_cell = iter;
		iter->previous_cell = task;

		if (task->previous_cell == NULL) {
			*list = task;
		}
		return MQX_OK;
	}

	// Otherwise figure out where we go
	while (iter->next_cell != NULL &&
			task->deadline.TICKS[1] <= iter->deadline.TICKS[1] &&
			task->deadline.TICKS[0] < iter->deadline.TICKS[0]) {
		iter = iter->next_cell;
	}

	if (iter->next_cell != NULL) {
		iter->next_cell->previous_cell = task;
	}
	iter->next_cell = task;
	task->previous_cell = iter;

	if (task->previous_cell == NULL) {
		*list = task;
	}

	return MQX_OK;
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

task_list_ptr delete_task(task_list_ptr * list, _task_id tid) {
	if (*list == NULL) {
		_task_set_error(MQX_INVALID_POINTER);
		return NULL;
	}

	task_list_ptr task = *list;

	while (task->tid != tid && task->next_cell != NULL) {
		task = task->next_cell;
	}
	if (task->tid != tid) {
		_task_set_error(MQX_INVALID_PARAMETER);
		return NULL;
	}

	if (task == *list) {
		*list = task->next_cell;
		if (task->next_cell != NULL) {
			task->next_cell->previous_cell = NULL;
		}
	} else {
		task_list_ptr p = task->previous_cell;
		if (p != NULL) {
			p->next_cell = task->next_cell;
		}
		if (task->next_cell != NULL) {
			task->next_cell->previous_cell = p;
		}
	}

	return task;
}

_mqx_uint sort_tasks(task_list_ptr list);

_mqx_uint update_priorities(task_list_ptr list) {
	_mqx_uint priority;
	_mqx_uint prev_priority;

	// get highest priority below scheduler task
	_task_get_priority(_task_get_id_from_name(GENERATOR_TASK_NAME), &priority);
	while (list != NULL) {
		// increase priority until min priority (run first so higher than gen)
		if (priority < _sched_get_min_priority(MQX_SCHED_FIFO)) {
			priority++;
		}

		_mqx_uint result = _task_set_priority(list->tid, priority, &prev_priority);
		if (result != MQX_OK) {
			// Task ended while we were updating priorities
			if (result != MQX_INVALID_TASK_ID) {
				return result;
			}
		}
		list = list->next_cell;
	}
	return MQX_OK;
}
