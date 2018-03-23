/*
 * dd_scheduler.c
 *
 *  Created on: Mar 17, 2018
 *      Author: Tayler Mulligan
 */

#include "dd_scheduler.h"

_pool_id scheduler_msg_pool;

/* ----------------------------------------------------------------------------
 * create_scheduler_request		Fill out boiler plate of a scheduler request msg
 * ----------------------------------------------------------------------------
 * Arguments:
 * 	_queue_id * q					Pointer to the variable holding the local
 * 										queue id
 * Returns:
 * 	SCHEDULER_RQST_MSG_PTR			Pointer to the partially filled out message
 *
 * Create a basic scheduler request message with constant fields filled in,
 * also sets the queue id used through `q`.
 */
static inline SCHEDULER_RQST_MSG_PTR create_scheduler_request(_queue_id * q) {
	*q = _msgq_open(MSGQ_FREE_QUEUE, 1);

	if (scheduler_msg_pool == 0) {
		scheduler_msg_pool = _msgpool_create(sizeof(SCHEDULER_RQST_MSG),
				INIT_MESSAGES, GROW_MESSAGES, 0);
	}
	SCHEDULER_RQST_MSG_PTR msg = _msg_alloc(scheduler_msg_pool);

	msg->HEADER.TARGET_QID = SCHEDULER_QID;
	msg->HEADER.SOURCE_QID = *q;
	msg->HEADER.SIZE = sizeof(SCHEDULER_RQST_MSG);

	return msg;
}

uint32_t dd_tcreate(_mqx_uint template_index, time_t deadline) {
	_queue_id local_q;
	SCHEDULER_RQST_MSG_PTR msg;

	msg = create_scheduler_request(&local_q);
	msg->RQST = CreateTask;
	msg->id = template_index;

	if (_msgq_send(msg)) {
		SCHEDULER_RESP_MSG_PTR resp =
				(SCHEDULER_RESP_MSG_PTR)_msgq_receive(local_q, TIMEOUT);
		if (resp) {
			_task_id id = resp->result;
			_msg_free(resp);
			return id;
		}
	}
	_msgq_close(local_q);
	return _task_get_error();
}

uint32_t dd_delete(_task_id task_id) {
	_queue_id local_q;
	SCHEDULER_RQST_MSG_PTR msg;
	uint32_t result;

	msg = create_scheduler_request(&local_q);
	msg->RQST = DeleteTask;
	msg->id = task_id;

	if (_msgq_send(msg)) {
		SCHEDULER_RESP_MSG_PTR resp =
				(SCHEDULER_RESP_MSG_PTR)_msgq_receive(local_q, TIMEOUT);
		if (resp) {
			result = resp->result;
			_msg_free(resp);
		}
	}
	_msgq_close(local_q);
	return result;
}

uint32_t dd_active_list(struct task_list **list) {
	_queue_id local_q;
	SCHEDULER_RQST_MSG_PTR msg;

	msg = create_scheduler_request(&local_q);
	msg->RQST = TaskList;

	uint32_t result;
	if (_msgq_send(msg)) {
		SCHEDULER_RESP_MSG_PTR resp =
				(SCHEDULER_RESP_MSG_PTR)_msgq_receive(local_q, TIMEOUT);
		if (resp) {
			result = resp->result;
			// If rqst okay, check if list pointer is valid
			if (result == MQX_OK) {
				if (resp->list != NULL) {
					*list = resp->list;
				} else {
					// If list pointer invalid, return error
					result = MQX_INVALID_POINTER;
				}
			}
			_msg_free(resp);
		}
	}
	_msgq_close(local_q);
	return result;
}

uint32_t dd_overdue_list(struct task_list **list) {
	_queue_id local_q;
	SCHEDULER_RQST_MSG_PTR msg;

	msg = create_scheduler_request(&local_q);
	msg->RQST = OverdueTaskList;

	uint32_t result;
	if (_msgq_send(msg)) {
		SCHEDULER_RESP_MSG_PTR resp =
				(SCHEDULER_RESP_MSG_PTR)_msgq_receive(local_q, TIMEOUT);
		if (resp) {
			result = resp->result;
			// If rqst okay, check if list pointer is valid
			if (result == MQX_OK) {
				if (resp->list != NULL) {
					*list = resp->list;
				} else {
					// If list pointer invalid, return error
					result = MQX_INVALID_POINTER;
				}
			}
			_msg_free(resp);
		}
	}
	_msgq_close(local_q);
	return result;
}
