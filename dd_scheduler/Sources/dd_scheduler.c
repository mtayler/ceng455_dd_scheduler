/*
 * dd_scheduler.c
 *
 *  Created on: Mar 17, 2018
 *      Author: Tayler Mulligan
 */

#include "dd_scheduler.h"

uint32_t dd_tcreate(_mqx_uint template_index, time_t deadline) {
	return 0;
}

uint32_t dd_delete(_task_id task_id) {
	return 0;
}

uint32_t dd_active_list(struct task_list **list) {
	if (tasks != NULL) {
		list = &tasks;
		return 0;
	} else {
		return MQX_INVALID_POINTER;
	}
}

uint32_t dd_overdue_list(struct overdue_task_list **list) {
	if (overdue_tasks != NULL) {
		list = &overdue_tasks;
		return 0;
	} else {
		return MQX_INVALID_POINTER;
	}
}
