/*
 * dd_scheduler.h
 *
 *  Created on: Mar 17, 2018
 *      Author: Tayler Mulligan
 */

#ifndef SOURCES_DD_SCHEDULER_H_
#define SOURCES_DD_SCHEDULER_H_

#include <stdint.h>
#include <stdio.h>
#include <mqx.h>
#include <message.h>
#include <mutex.h>

#include "task_list.h"

#define INIT_MESSAGES (4)
#define GROW_MESSAGES (2)

#define SCHEDULER_QID (3)

#define TIMEOUT (0)

#define debug(...) \
	printf("\n[%s]: ", __func__); \
	printf(__VA_ARGS__)

// Set by scheduler task on initialization
extern _pool_id scheduler_msg_pool;

enum REQUEST {
	CreateTask = 0,
	DeleteTask,
	TaskList,
	OverdueTaskList,
};

typedef struct scheduler_request_message {
	MESSAGE_HEADER_STRUCT HEADER;
	enum REQUEST RQST;
	uint32_t id; // template_index or task_id
	uint32_t parameter;
	uint32_t deadline;
} SCHEDULER_RQST_MSG, * SCHEDULER_RQST_MSG_PTR;

typedef struct scheduler_response_message {
	MESSAGE_HEADER_STRUCT HEADER;
	uint32_t result; // task_id
	uint32_t error;
	task_list_ptr list; // pointer to start of (overdue) task list for list requests
} SCHEDULER_RESP_MSG, * SCHEDULER_RESP_MSG_PTR;


/* ----------------------------------------------------------------------------
 * dd_tcreate		Create a new deadline-driven scheduled task
 * ----------------------------------------------------------------------------
 * Arguments:
 *  uint32_t template_index		Task template to use in task creation.
 *  uin32_t parameter			Parameter to pass task creation.
 * 	time_t deadline				Deadline of the task in absolute time.
 * Returns:
 * 	_task_id					Task ID of the task created or an error.
 *
 * Called by generating tasks to schedule a new periodic task.
 */
uint32_t dd_tcreate(uint32_t template_index, uint32_t parameter, time_t deadline);

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
 * 	task_list_ptr *list	Pointer to the pointer where the start of the task list
 * 							is assigned.
 * Returns:
 * 	uint32_t			MQX_OK if successful, otherwise an error
 *
 * Get a pointer to the start of the active task list
 */
uint32_t dd_active_list(task_list_ptr *list);

/* ----------------------------------------------------------------------------
 * dd_overdue_list		Return reference to list of overdue tasks
 * ----------------------------------------------------------------------------
 * Arguments:
 * 	task_list_ptr *list	Pointer to the pointer where the start of the overdue
 * 							task list is assigned.
 * Returns:
 * 	uint32_t			MQX_OK if successful, otherwise an error
 *
 * Get a pointer to the start of the overdue task list
 */
uint32_t dd_overdue_list(task_list_ptr *list);


#endif /* SOURCES_DD_SCHEDULER_H_ */
