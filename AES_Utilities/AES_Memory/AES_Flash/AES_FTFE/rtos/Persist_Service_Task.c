/*
 * Persist_Service_Task.c
 *
 *  Created on: Nov 16, 2018
 *      Author: AES_Local
 */

#include "FreeRTOS.h"
#include "semphr.h"

#include "portmacro.h"
#include "fsl_gpio.h"
#include "APEX_Hardware_Config.h"
#include "spsc_fifo.h"

#include <stdlib.h>      /* abs() */
#include "error_info.h"

#if ( 1 == configUSE_APPLICATION_TASK_TAG ) /* FreeRTOSConfig.h */
#include "tasks_top.h"
#endif

/*** < AES DEBUG - BEGIN > */
#if( 1 == AES_DEBUG_TOOL_SUITE_ENABLED )
/* SEGGER DEBUG ANALYSIS */
#include "AES_Debug/APEX_segger_track_control.h"
//#include "SEGGER/SEGGER_RTT.h"
#endif
/*** < AES DEBUG - END   > */

/*MACRO**********************************************************************
 * Description   : The Subversion (SVN) time/date marker which is updated
 * during commit of the source file to the repository.
 *END**************************************************************************/

#define SLEEP 0xFFFF
SemaphoreHandle_t Persist_Service_Semaphore_Handle = NULL;
#if ( 0 != configSUPPORT_STATIC_ALLOCATION )
StaticSemaphore_t Persist_Service_Semaphore_buffer;
#endif

TaskHandle_t Persist_Service_Task_Handle;

/*FUNCTION**********************************************************************
* Function Name :
* Description   :
*
*END**************************************************************************/
static void Task_Persist_Service( void *pvParameters )
{
    (void) pvParameters;

#define PEAK_OBJECT_SIZE    128

    char peak_buffer[PEAK_OBJECT_SIZE]; // 128 bytes

#if ( 1 == configUSE_APPLICATION_TASK_TAG ) /* FreeRTOSConfig.h */
    vTaskSetApplicationTaskTag( NULL, (void *) TASKS_TOP__PERSIST );
#endif

    uint32_t ulInterruptStatus;

    for( ;; )
    {
        /* Block indefinitely (without a timeout, so no need to check the function's
        return value) to wait for a notification.  NOTE!  Real applications
        should not block indefinitely, but instead time out occasionally in order
        to handle error conditions that may prevent the interrupt from sending
        any more notifications. */
        xTaskNotifyWait( 0x00,               /* Don't clear any bits on entry. */
                         ULONG_MAX,          /* Clear all bits on exit. */
                         &ulInterruptStatus, /* Receives the notification value. */
                         portMAX_DELAY );    /* Block indefinitely. */

        /* Process any bits set in the received notification value.  This assumes
        the peripheral sets bit 1 for an Rx interrupt, bit 2 for a Tx interrupt,
        and bit 3 for a buffer overrun interrupt. */
        if( ( ulInterruptStatus & WRITE_QUEUE_NOT_EMPTY ) != 0x00 )
        {
            //peak fifo
            peak(&Persist_Write_Queue, ((void *)&peak_buffer), PEAK_OBJECT_SIZE);


            //validate that the object in question fits in our peak buffer
            if( )
            {

            }

            //Invoke commit arbiter with the write queue with the record at hand

            //if succesfull commit
                //read fifo, don't do anything with the data
            //else
                //set flush commit fifo flag

        }

        if( ( ulInterruptStatus & COMMIT_QUEUE_NOT_EMPTY ) != 0x00 )
        {
            //Invoke commit arbiter with the commit queue
        }

    }
}

/*FUNCTION**********************************************************************
* Function Name :
* Description   :
*
*END**************************************************************************/
int Task_Persist_Service_Initialize( const struct task_shared_parameters * parameter,
                           void *                               additional_parameters,
                           TaskHandle_t *                       task_handle
                          )
{
    int         trouble;
    BaseType_t  result;
    (void)      additional_parameters;

    /* Create RTOS task */
    result = xTaskCreate(   Task_Persist_Service,                 /* task function */
                            parameter->name,        /* name of task */
                            parameter->stack_depth, /* stack space */
                            (void *) parameter,     /* constant parameters */
                            parameter->priority,    /* priority */
                            task_handle
                        );
    if ( pdPASS == result )
    {
        trouble = SUCCESS;
    }
    else
    {
        trouble = -EC_RTOS_TASK_INITIALIZE;
        goto TASK_PERSIST_SERVICE_INITIALIZE_ERROR_HANDLER;
    }

    /* assign the task handle */
    Persist_Service_Task_Handle = *task_handle;

    //create the binary semaphore
#if ( 0 != configSUPPORT_STATIC_ALLOCATION )
    Persist_Service_Semaphore_Handle = xSemaphoreCreateBinaryStatic( &Persist_Service_Semaphore_buffer );
#else /* configSUPPORT_DYNAMIC_ALLOCATION */
    Persist_Service_Semaphore_Handle = xSemaphoreCreateBinary();
#endif

    xSemaphoreGive(Persist_Service_Semaphore_Handle);

TASK_PERSIST_SERVICE_INITIALIZE_ERROR_HANDLER:
    return trouble;
}
