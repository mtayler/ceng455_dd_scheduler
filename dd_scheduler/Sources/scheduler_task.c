#include "scheduler_task.h"
#include "monitor_task.h"

#include <stdio.h>

#include "periodic_task.h"
#include "task_list.h"

#include "Cpu.h"
#include "Events.h"
#include "rtos_main_task.h"
#include "generator_task.h"
#include "dd_scheduler.h"

#define MAX_QUEUE_SIZE (0)

static void sched_create_task(SCHEDULER_RQST_MSG_PTR msg);
static void sched_delete_task(SCHEDULER_RQST_MSG_PTR msg);
static void sched_task_list(SCHEDULER_RQST_MSG_PTR msg);
static void sched_overdue_task_list(SCHEDULER_RQST_MSG_PTR msg);

static task_list_ptr tasks = NULL;
static task_list_ptr overdue_tasks = NULL;

static _pool_id scheduler_resp_pool;
static _queue_id scheduler_msg_q;

/*
** ===================================================================
**     Callback    : Scheduler_task
**     Description : Task function entry.
**     Parameters  :
**       task_init_data - OS task parameter
**     Returns : Nothing
** ===================================================================
*/
void Scheduler_task(os_task_param_t task_init_data)
{
	// Create the message pools
	scheduler_resp_pool = _msgpool_create(
			sizeof(SCHEDULER_RESP_MSG), INIT_MESSAGES, GROW_MESSAGES, 0);

	scheduler_msg_q = _msgq_open(SCHEDULER_QID, 0);

	_time_delay(5); // Allow other tasks to initialize
  
	while (1) {
		SCHEDULER_RQST_MSG_PTR msg = _msgq_receive(scheduler_msg_q, 0);

		if (msg) {
			switch(msg->RQST) {
				case CreateTask:
					sched_create_task(msg);
					break;
				case DeleteTask:
					sched_delete_task(msg);
					break;
				case TaskList:
					sched_task_list(msg);
					break;
				case OverdueTaskList:
					sched_overdue_task_list(msg);
					break;
				default:
					_msg_free(msg);
					SCHEDULER_RESP_MSG_PTR resp = _msg_alloc(scheduler_resp_pool);
					if (resp) {
						resp->HEADER.TARGET_QID = msg->HEADER.SOURCE_QID;
						resp->HEADER.SOURCE_QID = scheduler_msg_q;
						resp->HEADER.SIZE = sizeof(SCHEDULER_RESP_MSG);
						resp->result = MQX_EINVAL;

						_msgq_send(resp);
					}
					break;
			}
		} else {
			printf("Couldn't get message, error: %x", (uint)_task_get_error());
		}
	}
}

static void deadline_overdue(_timer_id timer, void * task,
		uint32_t mode, uint32_t time) {
	delete_task(&tasks, ((task_list_ptr)task)->tid);
	add_task(&overdue_tasks, (task_list_ptr)task);
}

static void sched_create_task(SCHEDULER_RQST_MSG_PTR msg) {
	// Create task
	_task_id tid = _task_create(MQX_NULL_TASK_ID, PERIODICTASK_TASK, msg->id);

	SCHEDULER_RESP_MSG_PTR resp = _msg_alloc(scheduler_resp_pool);
	resp->HEADER.TARGET_QID = msg->HEADER.SOURCE_QID;
	resp->HEADER.SOURCE_QID = scheduler_msg_q;
	resp->HEADER.SIZE = sizeof(SCHEDULER_RESP_MSG);

	TIME_STRUCT now;
	_time_get_elapsed(&now);

	task_list_ptr task = _mem_alloc(sizeof(task_list_t));
	task->tid = tid;
	task->deadline = (TIME_STRUCT){
		now.SECONDS, now.MILLISECONDS + periodic_tasks[msg->id].execution_time};
	task->task_type = 0;
	task->creation_time = now;

	_msg_free(msg);

	// Create a timer to check deadline
	_timer_id timer = _timer_start_oneshot_at(
			deadline_overdue, task,
			TIMER_ELAPSED_TIME_MODE, &(task->deadline));

	task->timer = timer;

	// Create task list entry
	if (add_task(&tasks, task) != NULL) {
		_mem_free(task);
		resp->result = _task_get_error();
		_msgq_send(resp);
		return;
	}

	// Assign priorities
	_mqx_uint result = assign_priorities(tasks);
	if (result != MQX_OK) {
		resp->result = result;
		_msgq_send(resp);
		return;
	}
}

static void sched_delete_task(SCHEDULER_RQST_MSG_PTR msg) {
	// Abort the task
	_task_abort(msg->id);

	SCHEDULER_RESP_MSG_PTR resp = _msg_alloc(scheduler_resp_pool);
	resp->HEADER.TARGET_QID = msg->HEADER.SOURCE_QID;
	resp->HEADER.SOURCE_QID = scheduler_msg_q;
	resp->HEADER.SIZE = sizeof(SCHEDULER_RESP_MSG);

	// Find task in active or overdue tasks and delete
	task_list_ptr task = get_task(tasks, msg->id);
	if (task == NULL) {
		task = get_task(overdue_tasks, msg->id);
		if (task != NULL) {
			delete_task(&tasks, task->tid);
		} else {
			_msg_free(msg);
			// No task was found
			resp->result = MQX_INVALID_PARAMETER;
			_msgq_send(resp);
			return;
		}
	} else {
		delete_task(&tasks, task->tid);
	}
	// Can free message now
	_msg_free(msg);

	// Delete any timers
	_timer_cancel(task->timer);

	// Assign priorities
	assign_priorities(tasks);
	assign_priorities(overdue_tasks);
}

static void sched_task_list(SCHEDULER_RQST_MSG_PTR msg) {
	SCHEDULER_RESP_MSG_PTR resp = _msg_alloc(scheduler_resp_pool);

	if (resp) {
		resp->HEADER.TARGET_QID = msg->HEADER.SOURCE_QID;
		resp->HEADER.SOURCE_QID = scheduler_msg_q;
		resp->list = tasks;
		resp->result = MQX_OK;

		_msgq_send(resp);
	}
	_msg_free(msg);

	return;
}

static void sched_overdue_task_list(SCHEDULER_RQST_MSG_PTR msg) {
	SCHEDULER_RESP_MSG_PTR resp = _msg_alloc(scheduler_resp_pool);

	if (resp) {
		resp->HEADER.TARGET_QID = msg->HEADER.SOURCE_QID;
		resp->HEADER.SOURCE_QID = scheduler_msg_q;
		resp->list = overdue_tasks;
		resp->result = MQX_OK;

		_msgq_send(resp);
	}
	_msg_free(msg);

	return;
}
