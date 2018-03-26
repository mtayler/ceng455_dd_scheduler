/*
 * task_list.c
 *
 *  Created on: Mar 22, 2018
 *      Author: mtayler
 */

#include "task_list.h"

#include <stdio.h>

#include "Generator.h"
#include "tick_funcs.h"

void add_task(task_list_ptr * list, task_list_ptr task) {
	assert(_mutex_lock(tasks_m) != MQX_EINVAL);
	task->next_cell = NULL;
	task->previous_cell = NULL;

	// I love sorting less than. So much fun.

	task_list_ptr iter = *list;

	// If list is empty we're the head
	if (iter == NULL) {
		*list = task;
		assert(_mutex_unlock(tasks_m) != MQX_EINVAL);
		return;
	}

	// If we're less than the first item we become the start
	if (TICKS_VAL(task->deadline.TICKS) < TICKS_VAL(iter->deadline.TICKS)) {
		task->next_cell = iter;
		task->previous_cell = iter->previous_cell;
		iter->previous_cell = task;

		*list = task;
		assert(_mutex_unlock(tasks_m) != MQX_EINVAL);
		return;
	}

	// Otherwise figure out what task we go after
	while (iter->next_cell != NULL &&
			(TICKS_VAL(task->deadline.TICKS) >
					TICKS_VAL(iter->next_cell->deadline.TICKS))) {
		iter = iter->next_cell;
	}

	task->previous_cell = iter;
	if (iter->next_cell != NULL) {
		iter->next_cell->previous_cell = task;
	}
	task->next_cell = iter->next_cell;
	iter->next_cell = task;

	assert(_mutex_unlock(tasks_m) != MQX_EINVAL);
	return;
}

task_list_ptr get_task(task_list_ptr list, _task_id tid) {
	assert(_mutex_lock(tasks_m) != MQX_EINVAL);
	task_list_ptr task = list;
	while (task->tid != tid && task->next_cell != NULL) {
		task = task->next_cell;
	}
	bool match = task-> tid == tid;
	assert(_mutex_unlock(tasks_m) != MQX_EINVAL);
	if (!match) {
		_task_set_error(MQX_INVALID_PARAMETER);
		return NULL;
	} else {
		return task;
	}
}

task_list_ptr delete_task(task_list_ptr * list, _task_id tid) {
	assert(_mutex_lock(tasks_m) != MQX_EINVAL);
	if (*list == NULL) {
		_task_set_error(MQX_INVALID_POINTER);
		return NULL;
	}

	task_list_ptr task = *list;

	while (task != NULL && task->tid != tid) {
		task = task->next_cell;
	}
	if (task == NULL) {
		_task_set_error(MQX_INVALID_PARAMETER);
		assert(_mutex_unlock(tasks_m) != MQX_EINVAL);
		return NULL;
	}

	// Bridge next and last cell if needed
	if (task->previous_cell != NULL) {
		task->previous_cell->next_cell = task->next_cell;
	} else {
		*list = task->next_cell;
	}
	if (task->next_cell != NULL) {
		task->next_cell->previous_cell = task->previous_cell;
	}
	assert(_mutex_lock(tasks_m) != MQX_EINVAL);
	return task;
}

_mqx_uint update_priorities(task_list_ptr list, uint32_t start_priority) {
	_mqx_uint priority = start_priority;
	_mqx_uint prev_priority;

	assert(_mutex_lock(tasks_m) != MQX_EINVAL);

	while (list != NULL) {
		// increase priority until min priority (run first so higher than gen)
		if (priority < _sched_get_min_priority(MQX_SCHED_FIFO)) {
			priority++;
		}

		_mqx_uint result = _task_set_priority(list->tid, priority, &prev_priority);
		if (result != MQX_OK) {
			assert(_mutex_unlock(tasks_m) != MQX_EINVAL);
			_task_set_error(result);
			return 0;
		}
		list = list->next_cell;
	}
	assert(_mutex_unlock(tasks_m) != MQX_EINVAL);
	return priority;
}
