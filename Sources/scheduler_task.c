#include "scheduler_task.h"
#include "monitor_task.h"

#include <stdio.h>
#include <stdarg.h>

#include <mutex.h>

#include "periodic_task.h"
#include "os_tasks.h"
#include "task_list.h"

#include "Cpu.h"
#include "Events.h"
#include "rtos_main_task.h"
#include "generator_task.h"
#include "dd_scheduler.h"

#define MAX_QUEUE_SIZE (0)

#define CHECK_RESULT (if (result != MQX_OK) { \
	resp->result = result; _msgq_send(resp); return; })

static void sched_create_task(SCHEDULER_RQST_MSG_PTR msg);
static void sched_delete_task(SCHEDULER_RQST_MSG_PTR msg);
static void sched_task_list(SCHEDULER_RQST_MSG_PTR msg);
static void sched_overdue_task_list(SCHEDULER_RQST_MSG_PTR msg);

static task_list_ptr active_tasks = NULL;
static task_list_ptr overdue_tasks = NULL;

static _pool_id scheduler_resp_pool;
static _queue_id scheduler_msg_q;

static MUTEX_ATTR_STRUCT task_m_attr = {
		.SCHED_PROTOCOL = MUTEX_PRIO_INHERIT,
		.VALID = TRUE,
		.WAIT_PROTOCOL = MUTEX_PRIORITY_QUEUEING,
};

// Synchronize between message responses and timer updates
static MUTEX_STRUCT tasks_m;



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
  _mutex_init(&tasks_m, &task_m_attr);

  // Create the message pools
  scheduler_resp_pool = _msgpool_create(
		  sizeof(SCHEDULER_RESP_MSG), INIT_MESSAGES, GROW_MESSAGES, 0);

  scheduler_msg_q = _msgq_open(SCHEDULER_QID, 0);
  
  SCHEDULER_RQST_MSG_PTR msg;

  // Overhead time tracking
  MQX_TICK_STRUCT start_ticks;
  MQX_TICK_STRUCT end_ticks;
  MQX_TICK_STRUCT diff_ticks;

  while (1) {
	// we're the highest priority task so don't worry about preemption
	_time_get_elapsed_ticks(&start_ticks);

	msg = _msgq_receive(scheduler_msg_q, 0);

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
	_time_get_elapsed_ticks(&end_ticks);
	_time_diff_ticks(&end_ticks, &start_ticks, &diff_ticks);
	monitor_add_overhead_ticks(&diff_ticks);
	_sched_yield();
  }
}

static void deadline_overdue(_timer_id timer, void * task,
		MQX_TICK_STRUCT_PTR time) {
	task_list_ptr t = (task_list_ptr)task;

	uint64_t now = (uint64_t)*(time->TICKS);
	uint32_t now_hw = time->HW_TICKS;
	uint64_t c = (uint64_t)*(t->creation_time.TICKS);
	uint32_t c_hw = t->creation_time.HW_TICKS;
	uint64_t dl = (uint64_t)*(t->creation_time.TICKS) + (uint64_t)*(t->deadline.TICKS);

	debug("(%llu.%lu) Task '%lu' created at %llu.%lu\n"
			"\twith type '%lu' is now overdue with deadline %llu.0000\n",
			now, now_hw, t->tid, c, c_hw, t->task_type, dl);

	// No timer watching task anymore
	t->timer = TIMER_NULL_ID;

	_mutex_lock(&tasks_m);
	task_list_ptr deleted_task = delete_task(&active_tasks, t->tid);
	if (deleted_task == NULL) {
		debug("Couldn't remove task from active tasks list\n");
	}
	add_task(&overdue_tasks, deleted_task);
	_mutex_unlock(&tasks_m);
}

static void sched_create_task(SCHEDULER_RQST_MSG_PTR msg) {
	_mqx_uint id = msg->id;
	uint32_t deadline = msg->deadline;

	SCHEDULER_RESP_MSG_PTR resp = _msg_alloc(scheduler_resp_pool);
	assert(resp != NULL);

	resp->HEADER.TARGET_QID = msg->HEADER.SOURCE_QID;
	resp->HEADER.SOURCE_QID = scheduler_msg_q;
	resp->HEADER.SIZE = sizeof(SCHEDULER_RESP_MSG);

	_msg_free(msg);

	// Create task
	_task_id tid = _task_create(0, PERIODICTASK_TASK, id);
	resp->result = tid;

	if (resp->result == MQX_NULL_TASK_ID) {
		resp->error = _task_get_error();
		_msgq_send(resp);
		return;
	}

	task_list_ptr task = _mem_alloc(sizeof(task_list_t));
	if (task == NULL) {
		resp->error = _task_get_error();
	}

	// Set creation and deadline ticks
	_time_get_elapsed_ticks(&(task->creation_time));
	task->tid = tid;
	*(task->deadline.TICKS) = (uint64_t)*(task->creation_time.TICKS)+deadline;
	task->deadline.HW_TICKS = 0;
	task->task_type = id;

	// Create a timer to check deadline
	_timer_id timer = _timer_start_oneshot_after_ticks(deadline_overdue, task,
			TIMER_ELAPSED_TIME_MODE, &(task->deadline));
	if (timer == TIMER_NULL_ID) {
		_mem_free(task);
		resp->error = _task_get_error();
		resp->result = MQX_NULL_TASK_ID;
		_msgq_send(resp);
		return;
	}
	task->timer = timer;

	// Create task list entry
	_mutex_lock(&tasks_m);
	add_task(&active_tasks, task);
	_mutex_unlock(&tasks_m);

	// Assign priorities
	_mutex_lock(&tasks_m);
	uint32_t status = update_priorities(active_tasks);
	_mutex_unlock(&tasks_m);

	resp->error = status;
	_msgq_send(resp);
	return;
}

static void sched_delete_task(SCHEDULER_RQST_MSG_PTR msg) {
	_mqx_uint id = msg->id;
	_mqx_uint result;

	SCHEDULER_RESP_MSG_PTR resp = _msg_alloc(scheduler_resp_pool);
	assert(resp != NULL);
	resp->HEADER.TARGET_QID = msg->HEADER.SOURCE_QID;
	resp->HEADER.SOURCE_QID = scheduler_msg_q;
	resp->HEADER.SIZE = sizeof(SCHEDULER_RESP_MSG);

	_msg_free(msg);

	// Find task in active or overdue active_tasks and delete
	_mutex_lock(&tasks_m);
	task_list_ptr task = delete_task(&active_tasks, id);
	if (task == NULL) {						// not in active_tasks
		result = _task_get_error();
		task = delete_task(&overdue_tasks, id);
		if (task == NULL) {
			result = _task_get_error();
		}
	}
	_mutex_unlock(&tasks_m);

	// Check if we found and deleted a task, otherwise stop
	if (task == NULL) {
		resp->result = result;
		_msgq_send(resp);
		return;
	}

	// Delete any timers
	if (task->timer != TIMER_NULL_ID) {
		result = _timer_cancel(task->timer);
		if (result != MQX_OK) {
			resp->result = result;
			_msgq_send(resp);
			return;
		}
	}

	// Free the task
	_mem_free(task);

	// Update priorities
	_mutex_lock(&tasks_m);
	result = update_priorities(overdue_tasks);
	if (result != MQX_OK) {
		resp->result = result;
		_msgq_send(resp);
		return;
	}
	result = update_priorities(active_tasks);
	_mutex_unlock(&tasks_m);

	resp->result = result;
	_msgq_send(resp);

	// Abort the task
	_task_abort(id);
}

static void sched_task_list(SCHEDULER_RQST_MSG_PTR msg) {
	SCHEDULER_RESP_MSG_PTR resp = _msg_alloc(scheduler_resp_pool);

	assert(resp != NULL);
	resp->HEADER.TARGET_QID = msg->HEADER.SOURCE_QID;
	resp->HEADER.SOURCE_QID = scheduler_msg_q;
	resp->HEADER.SIZE = sizeof(SCHEDULER_RESP_MSG);
	resp->list = active_tasks;
	resp->result = MQX_OK;

	_msg_free(msg);

	_msgq_send(resp);

	return;
}

static void sched_overdue_task_list(SCHEDULER_RQST_MSG_PTR msg) {
	SCHEDULER_RESP_MSG_PTR resp = _msg_alloc(scheduler_resp_pool);

	assert(resp != NULL);
	resp->HEADER.TARGET_QID = msg->HEADER.SOURCE_QID;
	resp->HEADER.SOURCE_QID = scheduler_msg_q;
	resp->HEADER.SIZE = sizeof(SCHEDULER_RESP_MSG);
	resp->list = overdue_tasks;
	resp->result = MQX_OK;

	_msg_free(msg);

	_msgq_send(resp);

	return;
}