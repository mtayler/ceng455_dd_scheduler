/*
 * dd_scheduler.h
 *
 *  Created on: Mar 17, 2018
 *      Author: Tayler Mulligan
 */

#ifndef SOURCES_DD_SCHEDULER_H_
#define SOURCES_DD_SCHEDULER_H_

#include <stdint.h>
#include <mqx.h>

// List of running tasks
struct task_list {
	uint32_t tid;
	uint32_t deadline;
	uint32_t task_type;
	uint32_t creation_time;
	struct task_list *next_cell;
	struct task_list *previous_cell;
} * tasks;

// List of overdue tasks
struct overdue_task_list {
	uint32_t tid;
	uint32_t deadline;
	uint32_t task_type;
	uint32_t creation_time;
	struct overdue_tasks *next_cell;
	struct overdue_tasks *previous_cell;
} * overdue_tasks;

/* ----------------------------------------------------------------------------
 * dd_tcreate		Create a new deadline-driven scheduled task
 * ----------------------------------------------------------------------------
 * Arguments:
 *  _mqx_uint template_index	Task template to use in task creation.
 * 	time_t deadline				Deadline of the task in absolute time.
 * Returns:
 * 	_task_id					Task ID of the task created or an error.
 *
 * Called by generating tasks to schedule a new periodic task.
 */
uint32_t dd_tcreate(_mqx_uint template_index, time_t deadline);

/* ----------------------------------------------------------------------------
 * dd_delete		Delete the task given by task_id
 * ----------------------------------------------------------------------------
 * Arguments:
 * 	_task_id task_id	ID of the task to delete.
 * Returns:
 * 	uint32_t			Task ID of the deleted task or an error.
 *
 * Called by a periodic function upon completion to remove itself from
 * the scheduler list.
 */
uint32_t dd_delete(_task_id task_id);

/* ----------------------------------------------------------------------------
 * dd_active_listq		Return reference to list of active tasks
 * ----------------------------------------------------------------------------
 * Arguments:
 * 	task_list **list	Pointer to the pointer where the start of the task list
 * 							is assigned.
 * Returns:
 * 	uint32_t			0 if successful, otherwise an error
 *
 * Get a pointer to the start of the active task list
 */
uint32_t dd_active_list(struct task_list **list);

/* ----------------------------------------------------------------------------
 * dd_overdue_list		Return reference to list of overdue tasks
 * ----------------------------------------------------------------------------
 * Arguments:
 * 	task_list **list	Pointer to the pointer where the start of the overdue
 * 							task list is assigned.
 * Returns:
 * 	uint32_t			0 if successful, otherwise an error
 *
 * Get a pointer to the start of the overdue task list
 */
uint32_t dd_overdue_list(struct overdue_task_list **list);


#endif /* SOURCES_DD_SCHEDULER_H_ */
