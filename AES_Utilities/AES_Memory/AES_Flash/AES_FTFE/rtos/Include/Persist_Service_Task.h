/*
 * Persist_Service_Task.h
 *
 *  Created on: Nov 16, 2018
 *      Author: AES_Local
 */

#ifndef PERSIST_SERVICE_TASK_H_
#define PERSIST_SERVICE_TASK_H_

#include "AES_Queue.h"

/* forward declaration */
int Task_Persist_Service_Initialize( const struct task_shared_parameters * parameter,
                           void *                               additional_parameters,
                           TaskHandle_t *                       task_handle
                          );

#endif /* PERSIST_SERVICE_TASK_H_ */
