/*
 * AES_Queue.h
 *
 *  Created on: Oct 5, 2018
 *      Author: AES_Local
 */

#ifndef SOURCE_AES_QUEUE_H_
#define SOURCE_AES_QUEUE_H_

#include "stdbool.h"


/***********************************************************************************************************************************************************
 *                                                                    QUEUE MACROS
 *
 *
 *
 *
 ************************************************************************************************************************************************************/

/***********************************************************************************************************************************************************
 *                                                                    QUEUE STRUCTURES
 *
 *
 *
 *
 ************************************************************************************************************************************************************/

typedef struct AES_Queue_Node_Data_Packet
{
	/* data */
    void * data;
    int data_size;
} AES_Queue_Node_Data_Packet;

typedef struct AES_Queue
{
	/* Queue heuristics */
	int Queue_number_of_nodes;
	int Queue_size_bytes;

    void *head, *tail;

} AES_Queue;

/***********************************************************************************************************************************************************
 *                                                                QUEUE PUBLIC FUNCTIONS
 *
 *
 *
 *
 ************************************************************************************************************************************************************/
void * AES_Queue_enQueue(AES_Queue * Queue, void * object, int object_size);
int AES_Queue_deQueue(AES_Queue * Queue, AES_Queue_Node_Data_Packet * data_packet);
AES_Queue_Node_Data_Packet * AES_Queue_Peak( AES_Queue * Queue);
int AES_Queue_Node_Count(AES_Queue * Queue);
bool AES_Queue_isEmpty( AES_Queue * Queue );
int AES_Queue_Delete( AES_Queue * Queue );
int AES_Queue_Print_Init( void * ptr );

#endif /* SOURCE_AES_QUEUE_H_ */
