/*
 * Persist_Idle_Task.c
 *
 *  Created on: Nov 16, 2018
 *      Author: AES_Local
 */

#include "FreeRTOS.h"
#include "AES_Queue.h"
#include "flash_persist_management.h"
#include "stdbool.h"

/*FUNCTION**********************************************************************
 * Function Name : vApplicationIdleHook
 * Description   : Invoked by the IDLE task. Checks to see if there are records
 * to be written within the Persist_Write_Queue. If there are, it will schedule
 * the Persist_Service_Task to run.
 *
 * parameters:
 * -void
 *
 * Return value:
 * -void
 *END**************************************************************************/
extern TaskHandle_t Persist_Service_Task_Handle;

void vApplicationIdleHook( void ){

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    bool write_queue_empty = true;
    bool commit_queue_empty = true;

    /* check if write queue is not empty */
    write_queue_empty = isEmpty((p_CircularQueue_t)&Persist_Write_Queue);

    /* check if commit queue is not empty */
    commit_queue_empty = isEmpty((p_CircularQueue_t)&Persist_Commit_Queue);

    /* wake the IDLE task */

    if(!write_queue_empty)
    {

        /* Notify the task that the transmission is complete by setting the TX_BIT
        in the task's notification value. */
        xTaskNotifyFromISR( Persist_Service_Task_Handle,
                            WRITE_QUEUE_NOT_EMPTY,
                            eSetBits,
                            &xHigherPriorityTaskWoken );

        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
        should be performed to ensure the interrupt returns directly to the highest
        priority task.  The macro used for this purpose is dependent on the port in
        use and may be called portEND_SWITCHING_ISR(). */
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }

    if(!commit_queue_empty)
    {

        /* Notify the task that the transmission is complete by setting the TX_BIT
        in the task's notification value. */
        xTaskNotifyFromISR( Persist_Service_Task_Handle,
                            COMMIT_QUEUE_NOT_EMPTY,
                            eSetBits,
                            &xHigherPriorityTaskWoken );

        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
        should be performed to ensure the interrupt returns directly to the highest
        priority task.  The macro used for this purpose is dependent on the port in
        use and may be called portEND_SWITCHING_ISR(). */
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }

    /* check if the bank balances cross the threshold */

    /* if so wake up the garbage collection task */

    /* else if either queue not empty, wake up the persist service task */


}


