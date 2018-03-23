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
	// Initialize mutexes
	_mutex_init(&active_mutex, &task_mutex_attr);
	_mutex_init(&overdue_mutex, &task_mutex_attr);
	// Create the message pools
	scheduler_resp_pool = _msgpool_create(
			sizeof(SCHEDULER_RESP_MSG), INIT_MESSAGES, GROW_MESSAGES, 0);

	scheduler_msg_q = _msgq_open(SCHEDULER_QID, 0);

	_sched_yield(); // Allow other tasks to initialize
  
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
		}
	}
}

static void deadline_overdue(_timer_id timer, void * task,
		uint32_t mode, uint32_t time) {
	_mutex_lock(&active_mutex);
	delete_task(&tasks, ((task_list_ptr)task)->tid);
	_mutex_unlock(&active_mutex);

	_mutex_lock(&overdue_mutex);
	add_task(&overdue_tasks, (task_list_ptr)task);
	_mutex_unlock(&overdue_mutex);
}

static void sched_create_task(SCHEDULER_RQST_MSG_PTR msg) {
	_mqx_uint id = msg->id;
	time_t deadline = msg->deadline;
	// Create task
	_task_id tid = _task_create(0, PERIODICTASK_TASK, id);

	SCHEDULER_RESP_MSG_PTR resp = _msg_alloc(scheduler_resp_pool);
	resp->HEADER.TARGET_QID = msg->HEADER.SOURCE_QID;
	resp->HEADER.SOURCE_QID = scheduler_msg_q;
	resp->HEADER.SIZE = sizeof(SCHEDULER_RESP_MSG);

	_msg_free(msg);

	if (tid == MQX_NULL_TASK_ID) {
		resp->result = _task_get_error();
		_msgq_send(resp);
		return;
	}

	task_list_ptr task = _mem_alloc(sizeof(task_list_t));
	if (task == NULL) {
		resp->result = _task_get_error();
		_msgq_send(resp);
		return;
	}

	TIME_STRUCT now;
	_time_get_elapsed(&now);

	task->tid = tid;
	task->deadline = (TIME_STRUCT){deadline, now.MILLISECONDS};
	task->task_type = 0;
	task->creation_time = now;

	// Create a timer to check deadline
	_timer_id timer = _timer_start_oneshot_at(
			deadline_overdue, task,
			TIMER_ELAPSED_TIME_MODE, &(task->deadline));

	task->timer = timer;

	// Create task list entry
	_mutex_lock(&active_mutex);
	_mqx_uint result = add_task(&tasks, task);
	_mutex_unlock(&active_mutex);
	if (result != MQX_OK) {
		_mem_free(task);
		resp->result = _task_get_error();
		_msgq_send(resp);
		return;
	}

	// Assign priorities
	_mutex_lock(&active_mutex);
	result = update_priorities(tasks);
	_mutex_unlock(&active_mutex);
	if (result != MQX_OK) {
		resp->result = result;
		_msgq_send(resp);
		return;
	}
}

static void sched_delete_task(SCHEDULER_RQST_MSG_PTR msg) {
	_mqx_uint id = msg->id;

	// Abort the task
	_task_abort(id);

	SCHEDULER_RESP_MSG_PTR resp = _msg_alloc(scheduler_resp_pool);
	resp->HEADER.TARGET_QID = msg->HEADER.SOURCE_QID;
	resp->HEADER.SOURCE_QID = scheduler_msg_q;
	resp->HEADER.SIZE = sizeof(SCHEDULER_RESP_MSG);

	_msg_free(msg);

	// Find task in active or overdue tasks and delete
	_mutex_lock(&active_mutex);
	task_list_ptr task = get_task(tasks, id);
	_mutex_unlock(&active_mutex);
	if (task == NULL) {
		if (_task_get_error() == MQX_INVALID_PARAMETER) {
			_mutex_lock(&overdue_mutex);
			task = get_task(overdue_tasks, id);
			_mutex_unlock(&overdue_mutex);
			if (task != NULL) {
				_mutex_lock(&overdue_mutex);
				delete_task(&overdue_tasks, task->tid);
				_mutex_unlock(&overdue_mutex);
			} else {
				// No task was found
				resp->result = MQX_INVALID_PARAMETER;
				_msgq_send(resp);
				return;
			}
		}
	} else {
		_mutex_lock(&active_mutex);
		delete_task(&tasks, task->tid);
		_mutex_unlock(&active_mutex);
	}

	// Delete any timers
	_timer_cancel(task->timer);

	// Assign priorities
	_mutex_lock(&active_mutex);
	update_priorities(tasks);
	_mutex_unlock(&active_mutex);

	_mutex_lock(&overdue_mutex);
	update_priorities(overdue_tasks);
	_mutex_unlock(&overdue_mutex);
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
